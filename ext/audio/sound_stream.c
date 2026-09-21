#include "audio/sound_stream.h"

#include <ruby.h>
#include <ruby/thread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio_enums.h"
#include "audio/effect_processor.h"
#include "audio/sound_source.h"
#include "core/exceptions.h"
#include "core/foreign_thread.h"
#include "system/time.h"

#define SOUND_STREAM_CALLBACK_TIMEOUT_MS 50

/* A subclassable stream. SFML calls the two C callbacks from its audio thread,
   which was never created by Ruby, so calling back into Ruby directly (e.g.
   via rb_thread_call_with_gvl) is a fatal VM error - only a genuine Ruby
   thread may reacquire the GVL that way. Each callback instead hands off to
   the shared foreign-thread worker (core/foreign_thread.h) and reports a
   timeout or exception as "stop the stream" rather than unwinding into the
   audio engine. The sample buffer lives in the wrapper and is reused across
   callbacks, because SFML keeps the pointer it is handed until the next
   onGetData. */
typedef struct {
    SoundSource source;
    VALUE rb_self;
    int16_t* samples;
    size_t samples_capacity;
} SoundStream;

static VALUE rb_cSoundStream;

static void SoundStream_mark(void* ptr) {
    SoundStream* stream = ptr;

    rb_gc_mark(stream->rb_self);
}

/* sfSoundStream_destroy() can block waiting on the audio thread, the same as
   Sound_free/SS_METHOD(stop); release the GVL for the same reason. */
static void* SoundStream_destroy_without_gvl(void* handle) {
    sfSoundStream_destroy(handle);
    return NULL;
}

static void SoundStream_free(void* ptr) {
    SoundStream* stream = ptr;

    /* Destroy (which blocks until CSFML guarantees no in-flight audio-thread
       callback still references this source) before releasing the effect
       slot, not after -- see the matching comment in sound.c's Sound_free. */
    rb_thread_call_without_gvl(SoundStream_destroy_without_gvl, stream->source.handle, RUBY_UBF_IO,
                               NULL);
    effect_processor_release(stream->source.effect_slot);
    free(stream->samples);
    free(stream);
}

/* Deliberately no RUBY_TYPED_FREE_IMMEDIATELY: SoundStream_free releases the
   GVL for sfSoundStream_destroy, and freeing during GC (which the flag
   requests) would let another thread allocate while GC is mid-cycle, which
   Ruby aborts on as "object allocation during garbage collection phase".
   Deferred finalization runs the same free with the GVL held and no GC in
   progress. */
static const rb_data_type_t SoundStream_data_type = {
    .wrap_struct_name = "SFML::SoundStream",
    .function = {.dmark = SoundStream_mark, .dfree = SoundStream_free, .dsize = NULL}};

/* Expands the stream's sample buffer if needed, then copies either a packed
   String of int16 or an Array of Integers into it. Returns the sample count. */
static size_t SoundStream_store_samples(SoundStream* stream, VALUE rb_samples) {
    size_t count;

    if (RB_TYPE_P(rb_samples, T_STRING)) {
        size_t bytes = (size_t)RSTRING_LEN(rb_samples);

        count = bytes / sizeof(int16_t);
    } else if (RB_TYPE_P(rb_samples, T_ARRAY)) {
        count = (size_t)RARRAY_LEN(rb_samples);
    } else {
        rb_raise(rb_eTypeError,
                 "on_get_data must return an Array of samples, a packed String, or nil");
        return 0;
    }

    if (count > stream->samples_capacity) {
        int16_t* grown = realloc(stream->samples, count * sizeof(int16_t));

        if (grown == NULL) {
            rb_raise(rb_eNoMemError, "could not grow sound stream buffer");
        }

        stream->samples = grown;
        stream->samples_capacity = count;
    }

    if (RB_TYPE_P(rb_samples, T_STRING)) {
        const int16_t* src = (const int16_t*)RSTRING_PTR(rb_samples);
        size_t i;

        for (i = 0; i < count; i++) {
            stream->samples[i] = src[i];
        }
    } else {
        size_t i;

        for (i = 0; i < count; i++) {
            stream->samples[i] = (int16_t)NUM2INT(rb_ary_entry(rb_samples, (long)i));
        }
    }

    return count;
}

typedef struct {
    SoundStream* stream;
    sfSoundStreamChunk* chunk;
    int ok;
} GetDataContext;

static VALUE SoundStream_get_data_body(VALUE raw) {
    GetDataContext* ctx = (GetDataContext*)raw;
    VALUE result = rb_funcall(ctx->stream->rb_self, rb_intern("on_get_data"), 0);

    if (NIL_P(result) || result == Qfalse) {
        ctx->ok = 0;
        return Qnil;
    }

    ctx->chunk->sampleCount = (unsigned int)SoundStream_store_samples(ctx->stream, result);
    ctx->chunk->samples = ctx->stream->samples;
    ctx->ok = 1;

    return Qnil;
}

static void SoundStream_get_data_run(void* raw) {
    int state = 0;

    rb_protect(SoundStream_get_data_body, (VALUE)raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        ((GetDataContext*)raw)->ok = 0;
    }
}

static bool SoundStream_on_get_data(sfSoundStreamChunk* chunk, void* userData) {
    SoundStream* stream = userData;
    GetDataContext ctx = {.stream = stream, .chunk = chunk, .ok = 0};

    /* On timeout ctx.ok stays 0, which reports "no data" below - the same
       fallback already used for a raised exception. */
    run_on_ruby_thread(SoundStream_get_data_run, &ctx, SOUND_STREAM_CALLBACK_TIMEOUT_MS);

    return ctx.ok ? true : false;
}

typedef struct {
    SoundStream* stream;
    sfTime time;
} SeekContext;

static VALUE SoundStream_seek_body(VALUE raw) {
    SeekContext* ctx = (SeekContext*)raw;

    rb_funcall(ctx->stream->rb_self, rb_intern("on_seek"), 1, time_to_rb(ctx->time));

    return Qnil;
}

static void SoundStream_seek_run(void* raw) {
    int state = 0;

    rb_protect(SoundStream_seek_body, (VALUE)raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
    }
}

static void SoundStream_on_seek(sfTime time, void* userData) {
    SoundStream* stream = userData;
    SeekContext ctx = {.stream = stream, .time = time};

    if (!rb_respond_to(stream->rb_self, rb_intern("on_seek"))) {
        return;
    }

    run_on_ruby_thread(SoundStream_seek_run, &ctx, SOUND_STREAM_CALLBACK_TIMEOUT_MS);
}

static VALUE SoundStream_alloc(VALUE klass) {
    SoundStream* ptr = malloc(sizeof(SoundStream));
    VALUE self;

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate sound stream");
    }

    ptr->source.handle = NULL;
    ptr->source.effect_slot = -1;
    ptr->rb_self = Qnil;
    ptr->samples = NULL;
    ptr->samples_capacity = 0;

    self = TypedData_Wrap_Struct(klass, &SoundStream_data_type, ptr);
    ptr->rb_self = self;

    return self;
}

/* call-seq:
 *   SoundStream.new(channel_count, sample_rate)             -> SoundStream
 *   SoundStream.new(channel_count, sample_rate, channel_map) -> SoundStream
 *
 * SoundStream must be subclassed: the subclass is required to implement
 * +#on_get_data+, called from a foreign audio thread whenever more samples
 * are needed, and may optionally implement +#on_seek+. +channel_map+, if
 * given, is an Array of channel Symbols (see SoundChannel), one per channel.
 *
 * @return [SoundStream]
 * @raise [NotImplementedError] if the subclass does not define #on_get_data
 * @raise [RuntimeError] if the underlying stream could not be created
 */
static VALUE SoundStream_initialize(int argc, VALUE* argv, VALUE self) {
    VALUE rb_channel_count, rb_sample_rate, rb_channel_map;
    sfSoundChannel* channel_map = NULL;
    size_t channel_map_size = 0;
    SoundStream* ptr;
    long i;

    TypedData_Get_Struct(self, SoundStream, &SoundStream_data_type, ptr);

    rb_scan_args(argc, argv, "21", &rb_channel_count, &rb_sample_rate, &rb_channel_map);

    if (!NIL_P(rb_channel_map)) {
        if (!RB_TYPE_P(rb_channel_map, T_ARRAY)) {
            rb_raise(rb_eArgError, "channel map must be an Array");
        }

        channel_map_size = (size_t)RARRAY_LEN(rb_channel_map);
        channel_map = malloc(sizeof(sfSoundChannel) * channel_map_size);

        for (i = 0; i < (long)channel_map_size; i++) {
            channel_map[i] = sound_channel_from_rb(rb_ary_entry(rb_channel_map, i));
        }
    }

    if (!rb_respond_to(self, rb_intern("on_get_data"))) {
        free(channel_map);
        rb_raise(rb_eNotImpError, "subclass must define #on_get_data");
    }

    ptr->source.handle = sfSoundStream_create(
        SoundStream_on_get_data, SoundStream_on_seek, (unsigned int)NUM2INT(rb_channel_count),
        (unsigned int)NUM2INT(rb_sample_rate), channel_map, channel_map_size, ptr);

    free(channel_map);

    if (ptr->source.handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound stream");
    }

    return self;
}

/* call-seq: channel_count -> Integer
 *
 * Returns the number of audio channels the stream produces.
 *
 * @return [Integer]
 */
static VALUE SoundStream_channel_count(VALUE self) {
    return UINT2NUM(sfSoundStream_getChannelCount(Get_SoundStream_Struct(self)));
}

/* call-seq: sample_rate -> Integer
 *
 * Returns the stream's sample rate in samples per second.
 *
 * @return [Integer]
 */
static VALUE SoundStream_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundStream_getSampleRate(Get_SoundStream_Struct(self)));
}

/* call-seq: channel_map -> Array<Symbol>
 *
 * Returns the channel layout of the stream.
 *
 * @return [Array<Symbol>] one entry per channel, e.g.
 *   +[:front_left, :front_right]+
 */
static VALUE SoundStream_channel_map(VALUE self) {
    size_t count = 0;
    sfSoundChannel* map = sfSoundStream_getChannelMap(Get_SoundStream_Struct(self), &count);
    VALUE rb_array;
    size_t i;

    if (map == NULL) {
        return rb_ary_new();
    }

    rb_array = rb_ary_new_capa((long)count);

    for (i = 0; i < count; i++) {
        rb_ary_push(rb_array, ID2SYM(rb_intern(sound_channel_name(map[i]))));
    }

    return rb_array;
}

#define SS_FN(name) sfSoundStream_##name
#define SS_METHOD(name) SoundStream_##name
#include "audio/sound_source.inc"
#undef SS_FN
#undef SS_METHOD

/* Document-class: SFML::SoundStream
 * Base class for a custom audio source that generates or decodes its own
 * samples on demand. Derives from SoundSource; Music derives from it.
 * Subclasses must implement +#on_get_data+, returning an
 * Array of Integer samples or a packed String of int16 samples for the next
 * chunk (or +nil+/+false+ to signal end of stream), and may implement
 * +#on_seek(time)+ to support seeking.
 *
 * @!method play!
 *   Starts playback, or resumes it when paused. +play+ is a deprecated alias.
 *   @return [self]
 * @!method pause!
 *   Pauses playback, keeping the current playing offset. +pause+ is a
 *   deprecated alias.
 *   @return [self]
 * @!method stop!
 *   Stops playback and rewinds to the beginning. May briefly block the
 *   calling thread if an audio-thread callback for this source is in
 *   flight. +stop+ is a deprecated alias.
 *   @return [self]
 * @!method status
 *   Returns the current playback status.
 *   @return [Symbol] one of +:stopped+, +:paused+, +:playing+
 * @!method looping?
 *   Returns +true+ if playback loops back to the start on completion.
 *   @return [Boolean]
 * @!method looping=(value)
 *   Enables or disables looping.
 *   @return [Boolean]
 * @!method pitch
 *   Returns the pitch scaling factor.
 *   @return [Float]
 * @!method pitch=(value)
 *   Sets the pitch scaling factor.
 *   @return [Float]
 * @!method pan
 *   Returns the source's stereo pan.
 *   @return [Float] stereo pan, -1 (left) to 1 (right)
 * @!method pan=(value)
 *   Sets the source's stereo pan.
 *   @return [Float]
 * @!method volume
 *   Returns the source's volume.
 *   @return [Float] 0 to 100
 * @!method volume=(value)
 *   Sets the source's volume.
 *   @return [Float]
 * @!method spatialization_enabled?
 *   Returns +true+ if 3D spatialization is enabled.
 *   @return [Boolean]
 * @!method spatialization_enabled=(value)
 *   Enables or disables 3D spatialization.
 *   @return [Boolean]
 * @!method position
 *   Returns the source's position in 3D space.
 *   @return [Vector3]
 * @!method position=(value)
 *   Sets the source's position in 3D space.
 *   @return [Vector3]
 * @!method direction
 *   Returns the direction the source is facing.
 *   @return [Vector3]
 * @!method direction=(value)
 *   Sets the direction the source is facing.
 *   @return [Vector3]
 * @!method velocity
 *   Returns the source's velocity, used for Doppler calculations.
 *   @return [Vector3]
 * @!method velocity=(value)
 *   Sets the source's velocity for Doppler calculations.
 *   @return [Vector3]
 * @!method cone
 *   Returns the source's directional attenuation cone.
 *   @return [SoundSourceCone]
 * @!method cone=(value)
 *   Sets the source's directional attenuation cone.
 *   @return [SoundSourceCone]
 * @!method doppler_factor
 *   Returns the factor by which the Doppler effect is scaled.
 *   @return [Float]
 * @!method doppler_factor=(value)
 *   Sets the factor by which the Doppler effect is scaled.
 *   @return [Float]
 * @!method directional_attenuation_factor
 *   Returns the factor controlling directional attenuation.
 *   @return [Float]
 * @!method directional_attenuation_factor=(value)
 *   Sets the factor controlling directional attenuation.
 *   @return [Float]
 * @!method relative_to_listener?
 *   Returns +true+ if the source is positioned relative to the listener.
 *   @return [Boolean]
 * @!method relative_to_listener=(value)
 *   Makes the source relative to, or independent of, the listener.
 *   @return [Boolean]
 * @!method min_distance
 *   Returns the minimum distance of the distance-attenuation model.
 *   @return [Float]
 * @!method min_distance=(value)
 *   Sets the minimum distance of the distance-attenuation model.
 *   @return [Float]
 * @!method max_distance
 *   Returns the maximum distance of the distance-attenuation model.
 *   @return [Float]
 * @!method max_distance=(value)
 *   Sets the maximum distance of the distance-attenuation model.
 *   @return [Float]
 * @!method min_gain
 *   Returns the minimum gain of the distance-attenuation model.
 *   @return [Float]
 * @!method min_gain=(value)
 *   Sets the minimum gain of the distance-attenuation model.
 *   @return [Float]
 * @!method max_gain
 *   Returns the maximum gain of the distance-attenuation model.
 *   @return [Float]
 * @!method max_gain=(value)
 *   Sets the maximum gain of the distance-attenuation model.
 *   @return [Float]
 * @!method attenuation
 *   Returns the distance-attenuation factor.
 *   @return [Float]
 * @!method attenuation=(value)
 *   Sets the distance-attenuation factor.
 *   @return [Float]
 * @!method playing_offset
 *   Returns the current playing offset.
 *   @return [Time]
 * @!method playing_offset=(value)
 *   Seeks to the given playing offset.
 *   @return [Time]
 * @!method effect_processor=(proc)
 *   Installs a Proc that post-processes this source's audio in real time.
 *   @return [Proc] +proc+
 */
void Init_SoundStream(VALUE rb_mSFML) {
    rb_cSoundStream = rb_define_class_under(rb_mSFML, "SoundStream", Get_Klass_SoundSource());

    rb_define_alloc_func(rb_cSoundStream, SoundStream_alloc);
    rb_define_method(rb_cSoundStream, "initialize", SoundStream_initialize, -1);

    rb_define_method(rb_cSoundStream, "channel_count", SoundStream_channel_count, 0);
    rb_define_method(rb_cSoundStream, "sample_rate", SoundStream_sample_rate, 0);
    rb_define_method(rb_cSoundStream, "channel_map", SoundStream_channel_map, 0);

    SoundStream_define_sound_source_methods(rb_cSoundStream);
}

VALUE Get_Klass_SoundStream(void) {
    return rb_cSoundStream;
}

void* Get_SoundStream_Struct(VALUE self) {
    SoundStream* ptr;
    TypedData_Get_Struct(self, SoundStream, &SoundStream_data_type, ptr);
    return ptr->source.handle;
}

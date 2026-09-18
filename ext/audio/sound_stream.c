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

    effect_processor_release(stream->source.effect_slot);
    rb_thread_call_without_gvl(SoundStream_destroy_without_gvl, stream->source.handle, RUBY_UBF_IO,
                               NULL);
    free(stream->samples);
    free(stream);
}

static const rb_data_type_t SoundStream_data_type = {
    .wrap_struct_name = "SFML::SoundStream",
    .function = {.dmark = SoundStream_mark, .dfree = SoundStream_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

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
static VALUE SoundStream_new(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_channel_count, rb_sample_rate, rb_channel_map;
    sfSoundChannel* channel_map = NULL;
    size_t channel_map_size = 0;
    SoundStream* ptr;
    VALUE self;
    long i;

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

    ptr = malloc(sizeof(SoundStream));
    ptr->source.handle = NULL;
    ptr->source.effect_slot = -1;
    ptr->rb_self = Qnil;
    ptr->samples = NULL;
    ptr->samples_capacity = 0;

    self = TypedData_Wrap_Struct(klass, &SoundStream_data_type, ptr);
    ptr->rb_self = self;

    if (!rb_respond_to(self, rb_intern("on_get_data"))) {
        /* The wrapper is already owned by Ruby; its dfree tolerates a NULL
           handle, so raising here just leaves it to be collected. */
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
 * @return [Integer]
 */
static VALUE SoundStream_channel_count(VALUE self) {
    return UINT2NUM(sfSoundStream_getChannelCount(Get_SoundStream_Struct(self)));
}

/* call-seq: sample_rate -> Integer
 *
 * @return [Integer]
 */
static VALUE SoundStream_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundStream_getSampleRate(Get_SoundStream_Struct(self)));
}

/* call-seq: channel_map -> Array<Symbol>
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
 * samples on demand. Subclasses must implement +#on_get_data+, returning an
 * Array of Integer samples or a packed String of int16 samples for the next
 * chunk (or +nil+/+false+ to signal end of stream), and may implement
 * +#on_seek(time)+ to support seeking.
 *
 * @!method play
 *   @return [self]
 * @!method pause
 *   @return [self]
 * @!method stop
 *   Stops playback and rewinds to the beginning. May briefly block the
 *   calling thread if an audio-thread callback for this source is in
 *   flight.
 *   @return [self]
 * @!method status
 *   @return [Symbol] one of +:stopped+, +:paused+, +:playing+
 * @!method looping?
 *   @return [Boolean]
 * @!method looping=(value)
 *   @return [Boolean]
 * @!method pitch
 *   @return [Float]
 * @!method pitch=(value)
 *   @return [Float]
 * @!method pan
 *   @return [Float] stereo pan, -1 (left) to 1 (right)
 * @!method pan=(value)
 *   @return [Float]
 * @!method volume
 *   @return [Float] 0 to 100
 * @!method volume=(value)
 *   @return [Float]
 * @!method spatialization_enabled?
 *   @return [Boolean]
 * @!method spatialization_enabled=(value)
 *   @return [Boolean]
 * @!method position
 *   @return [Vector3]
 * @!method position=(value)
 *   @return [Vector3]
 * @!method direction
 *   @return [Vector3]
 * @!method direction=(value)
 *   @return [Vector3]
 * @!method velocity
 *   @return [Vector3]
 * @!method velocity=(value)
 *   @return [Vector3]
 * @!method cone
 *   @return [SoundSourceCone]
 * @!method cone=(value)
 *   @return [SoundSourceCone]
 * @!method doppler_factor
 *   @return [Float]
 * @!method doppler_factor=(value)
 *   @return [Float]
 * @!method directional_attenuation_factor
 *   @return [Float]
 * @!method directional_attenuation_factor=(value)
 *   @return [Float]
 * @!method relative_to_listener?
 *   @return [Boolean]
 * @!method relative_to_listener=(value)
 *   @return [Boolean]
 * @!method min_distance
 *   @return [Float]
 * @!method min_distance=(value)
 *   @return [Float]
 * @!method max_distance
 *   @return [Float]
 * @!method max_distance=(value)
 *   @return [Float]
 * @!method min_gain
 *   @return [Float]
 * @!method min_gain=(value)
 *   @return [Float]
 * @!method max_gain
 *   @return [Float]
 * @!method max_gain=(value)
 *   @return [Float]
 * @!method attenuation
 *   @return [Float]
 * @!method attenuation=(value)
 *   @return [Float]
 * @!method playing_offset
 *   @return [Time]
 * @!method playing_offset=(value)
 *   @return [Time]
 * @!method effect_processor=(proc)
 *   Installs a Proc that post-processes this source's audio in real time.
 *   @return [Proc] +proc+
 */
void Init_SoundStream(VALUE rb_mSFML) {
    rb_cSoundStream = rb_define_class_under(rb_mSFML, "SoundStream", rb_cObject);

    rb_define_singleton_method(rb_cSoundStream, "new", SoundStream_new, -1);

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

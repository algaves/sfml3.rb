#include "audio/music.h"

#include <ruby.h>
#include <ruby/thread.h>
#include <stdlib.h>

#include "audio/audio_enums.h"
#include "audio/effect_processor.h"
#include "audio/sound_source.h"
#include "core/exceptions.h"
#include "system/input_stream.h"
#include "system/time.h"

typedef struct {
    SoundSource source;
    /* Music streams from the InputStream lazily, so unlike Font/Image/Shader
       the stream has to outlive the create call. The VALUE is both marked and
       registered as a GC root: the root is what guarantees the InputStream's
       sfInputStream is still alive when sfMusic_destroy runs in dfree, even if
       the Music and the stream become unreachable in the same GC cycle. */
    VALUE rb_stream;
} Music;

static VALUE rb_cMusic;

static void Music_mark(void* ptr) {
    Music* music = ptr;

    rb_gc_mark(music->rb_stream);
}

/* sfMusic_destroy() calls Music::~Music(), which explicitly calls stop() and
   can block waiting on the audio thread; release the GVL for the same reason
   as Sound_free/SS_METHOD(stop). */
static void* Music_destroy_without_gvl(void* handle) {
    sfMusic_destroy(handle);
    return NULL;
}

static void Music_free(void* ptr) {
    Music* music = ptr;

    /* Destroy (which blocks until CSFML guarantees no in-flight audio-thread
       callback still references this source) before releasing the effect
       slot, not after -- see the matching comment in sound.c's Sound_free. */
    rb_thread_call_without_gvl(Music_destroy_without_gvl, music->source.handle, RUBY_UBF_IO, NULL);
    effect_processor_release(music->source.effect_slot);
    rb_gc_unregister_address(&music->rb_stream);
    free(music);
}

/* Deliberately no RUBY_TYPED_FREE_IMMEDIATELY: Music_free releases the GVL for
   sfMusic_destroy, and freeing during GC (which the flag requests) would let
   another thread allocate while GC is mid-cycle, which Ruby aborts on as
   "object allocation during garbage collection phase". Deferred finalization
   runs the same free with the GVL held and no GC in progress. */
static const rb_data_type_t Music_data_type = {
    .wrap_struct_name = "SFML::Music",
    .function = {.dmark = Music_mark, .dfree = Music_free, .dsize = NULL}};

static VALUE Music_wrap(VALUE klass, sfMusic* handle, VALUE rb_stream) {
    Music* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create music");
    }

    ptr = malloc(sizeof(Music));
    ptr->source.handle = handle;
    ptr->source.effect_slot = -1;
    ptr->rb_stream = rb_stream;

    rb_gc_register_address(&ptr->rb_stream);

    return TypedData_Wrap_Struct(klass, &Music_data_type, ptr);
}

/* call-seq:
 *   Music.from_file(path) -> Music
 *
 * Creates a music stream that reads from an audio file.
 *
 * @return [Music]
 * @raise [RuntimeError] if the file cannot be opened or decoded
 */
static VALUE Music_from_file(VALUE klass, VALUE rb_path) {
    return Music_wrap(klass, sfMusic_createFromFile(StringValueCStr(rb_path)), Qnil);
}

/* call-seq:
 *   Music.from_memory(data) -> Music
 *
 * Creates a music stream that reads from audio held in a String.
 *
 * @return [Music]
 * @raise [RuntimeError] if +data+ cannot be decoded
 */
static VALUE Music_from_memory(VALUE klass, VALUE rb_data) {
    StringValue(rb_data);

    return Music_wrap(
        klass, sfMusic_createFromMemory(RSTRING_PTR(rb_data), (size_t)RSTRING_LEN(rb_data)), Qnil);
}

/* call-seq:
 *   Music.from_stream(stream) -> Music
 *
 * Creates a music stream that reads from a custom InputStream.
 *
 * @return [Music]
 * @raise [RuntimeError] if the stream cannot be decoded
 */
static VALUE Music_from_stream(VALUE klass, VALUE rb_stream) {
    VALUE holder = Qnil;
    sfInputStream* stream = input_stream_from_rb(rb_stream, &holder);

    return Music_wrap(klass, sfMusic_createFromStream(stream), holder);
}

/* call-seq: duration -> Time
 *
 * Returns the total duration of the music.
 *
 * @return [Time] total duration of the music
 */
static VALUE Music_duration(VALUE self) {
    return time_to_rb(sfMusic_getDuration(Get_Music_Struct(self)));
}

/* call-seq: channel_count -> Integer
 *
 * Returns the number of audio channels in the music.
 *
 * @return [Integer]
 */
static VALUE Music_channel_count(VALUE self) {
    return UINT2NUM(sfMusic_getChannelCount(Get_Music_Struct(self)));
}

/* call-seq: sample_rate -> Integer
 *
 * Returns the music's sample rate in samples per second.
 *
 * @return [Integer]
 */
static VALUE Music_sample_rate(VALUE self) {
    return UINT2NUM(sfMusic_getSampleRate(Get_Music_Struct(self)));
}

/* call-seq: channel_map -> Array<Symbol>
 *
 * Returns the channel layout of the music.
 *
 * @return [Array<Symbol>] one entry per channel, e.g.
 *   +[:front_left, :front_right]+
 */
static VALUE Music_channel_map(VALUE self) {
    size_t count = 0;
    const sfSoundChannel* map = sfMusic_getChannelMap(Get_Music_Struct(self), &count);
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

/* Loop points are an offset plus a length, matching sfTimeSpan. Returned as a
   two-element [offset, length] pair of SFML::Time. */
/* call-seq: loop_points -> [Time, Time]
 *
 * Returns the loop points as an +[offset, length]+ pair.
 *
 * @return [Array<Time>] a two-element +[offset, length]+ pair
 */
static VALUE Music_loop_points(VALUE self) {
    sfTimeSpan span = sfMusic_getLoopPoints(Get_Music_Struct(self));

    return rb_ary_new_from_args(2, time_to_rb(span.offset), time_to_rb(span.length));
}

/* call-seq:
 *   loop_points=(value) -> [Time, Time]
 *
 * Sets the loop points as a two-element +[offset, length]+ pair.
 *
 * @return [Array<Time>] +value+
 * @raise [ArgumentError] if +value+ is not a two-element Array
 */
static VALUE Music_set_loop_points(VALUE self, VALUE rb_span) {
    sfTimeSpan span;

    if (!RB_TYPE_P(rb_span, T_ARRAY) || RARRAY_LEN(rb_span) != 2) {
        rb_raise(rb_eArgError, "loop points must be a two-element [offset, length] array");
    }

    span.offset = time_from_rb(rb_ary_entry(rb_span, 0));
    span.length = time_from_rb(rb_ary_entry(rb_span, 1));

    sfMusic_setLoopPoints(Get_Music_Struct(self), span);

    return rb_span;
}

#define SS_FN(name) sfMusic_##name
#define SS_METHOD(name) Music_##name
#include "audio/sound_source.inc"
#undef SS_FN
#undef SS_METHOD

/* Document-class: SFML::Music
 * Music streamed from a file, memory buffer or InputStream rather than held
 * fully decoded in memory, so it's suited to long tracks that would be
 * wasteful to load whole as a SoundBuffer.
 *
 * @!method play
 *   Starts playback, or resumes it when paused.
 *   @return [self]
 * @!method pause
 *   Pauses playback, keeping the current playing offset.
 *   @return [self]
 * @!method stop
 *   Stops playback and rewinds to the beginning. May briefly block the
 *   calling thread if an audio-thread callback for this source is in
 *   flight.
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
void Init_Music(VALUE rb_mSFML) {
    rb_cMusic = rb_define_class_under(rb_mSFML, "Music", rb_cObject);

    rb_define_singleton_method(rb_cMusic, "from_file", Music_from_file, 1);
    rb_define_singleton_method(rb_cMusic, "from_memory", Music_from_memory, 1);
    rb_define_singleton_method(rb_cMusic, "from_stream", Music_from_stream, 1);

    rb_define_method(rb_cMusic, "duration", Music_duration, 0);
    rb_define_method(rb_cMusic, "channel_count", Music_channel_count, 0);
    rb_define_method(rb_cMusic, "sample_rate", Music_sample_rate, 0);
    rb_define_method(rb_cMusic, "channel_map", Music_channel_map, 0);
    rb_define_method(rb_cMusic, "loop_points", Music_loop_points, 0);
    rb_define_method(rb_cMusic, "loop_points=", Music_set_loop_points, 1);

    Music_define_sound_source_methods(rb_cMusic);
}

VALUE Get_Klass_Music(void) {
    return rb_cMusic;
}

void* Get_Music_Struct(VALUE self) {
    Music* ptr;
    TypedData_Get_Struct(self, Music, &Music_data_type, ptr);
    return ptr->source.handle;
}

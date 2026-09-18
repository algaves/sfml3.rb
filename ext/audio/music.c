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

static const rb_data_type_t Music_data_type = {
    .wrap_struct_name = "SFML::Music",
    .function = {.dmark = Music_mark, .dfree = Music_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

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
 * @return [Music]
 * @raise [RuntimeError] if the file cannot be opened or decoded
 */
static VALUE Music_from_file(VALUE klass, VALUE rb_path) {
    return Music_wrap(klass, sfMusic_createFromFile(StringValueCStr(rb_path)), Qnil);
}

/* call-seq:
 *   Music.from_memory(data) -> Music
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
 * @return [Time] total duration of the music
 */
static VALUE Music_duration(VALUE self) {
    return time_to_rb(sfMusic_getDuration(Get_Music_Struct(self)));
}

/* call-seq: channel_count -> Integer
 *
 * @return [Integer]
 */
static VALUE Music_channel_count(VALUE self) {
    return UINT2NUM(sfMusic_getChannelCount(Get_Music_Struct(self)));
}

/* call-seq: sample_rate -> Integer
 *
 * @return [Integer]
 */
static VALUE Music_sample_rate(VALUE self) {
    return UINT2NUM(sfMusic_getSampleRate(Get_Music_Struct(self)));
}

/* call-seq: channel_map -> Array<Symbol>
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

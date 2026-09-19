#include "audio/sound.h"

#include <ruby.h>
#include <ruby/thread.h>
#include <stdlib.h>

#include "audio/effect_processor.h"
#include "audio/sound_buffer.h"
#include "audio/sound_source.h"
#include "core/exceptions.h"

typedef struct {
    SoundSource source;
    VALUE rb_buffer;
} Sound;

static VALUE rb_cSound;

static void Sound_mark(void* ptr) {
    Sound* sound = ptr;

    rb_gc_mark(sound->rb_buffer);
}

/* sfSound_destroy() calls Sound::stop(), which can block waiting on the audio
   thread (see the matching comment on SS_METHOD(stop) in sound_source.inc);
   release the GVL for the same reason. */
static void* Sound_destroy_without_gvl(void* handle) {
    sfSound_destroy(handle);
    return NULL;
}

static void Sound_free(void* ptr) {
    Sound* sound = ptr;

    /* Destroy (which blocks until CSFML guarantees no in-flight audio-thread
       callback still references this source) before releasing the effect
       slot, not after: releasing first opens a window where another Ruby
       thread's effect_processor_acquire could reuse the slot number for an
       unrelated new source while this source's callback might still be
       executing against it. */
    rb_thread_call_without_gvl(Sound_destroy_without_gvl, sound->source.handle, RUBY_UBF_IO, NULL);
    effect_processor_release(sound->source.effect_slot);
    free(sound);
}

static const rb_data_type_t Sound_data_type = {
    .wrap_struct_name = "SFML::Sound",
    .function = {.dmark = Sound_mark, .dfree = Sound_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Sound_wrap(VALUE klass, sfSound* handle, VALUE rb_buffer) {
    Sound* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound");
    }

    ptr = malloc(sizeof(Sound));
    ptr->source.handle = handle;
    ptr->source.effect_slot = -1;
    ptr->rb_buffer = rb_buffer;

    return TypedData_Wrap_Struct(klass, &Sound_data_type, ptr);
}

/* call-seq:
 *   Sound.new(buffer) -> Sound
 *
 * @return [Sound]
 * @raise [ArgumentError] if +buffer+ is not a SoundBuffer
 */
static VALUE Sound_new(VALUE klass, VALUE rb_buffer) {
    if (!rb_obj_is_kind_of(rb_buffer, Get_Klass_SoundBuffer())) {
        raise_invalid_argument_class(Get_Klass_SoundBuffer());
    }

    return Sound_wrap(klass, sfSound_create(Get_SoundBuffer_Struct(rb_buffer)), rb_buffer);
}

/* call-seq: copy -> Sound
 *
 * @return [Sound] an independent copy that shares the same SoundBuffer
 */
static VALUE Sound_copy(VALUE self) {
    Sound* sound = (Sound*)Get_Sound_Struct(self);

    return Sound_wrap(Get_Klass_Sound(), sfSound_copy((const sfSound*)sound->source.handle),
                      sound->rb_buffer);
}

/* call-seq: buffer -> SoundBuffer
 *
 * @return [SoundBuffer] the buffer currently attached to this sound
 */
static VALUE Sound_get_buffer(VALUE self) {
    return ((Sound*)Get_Sound_Struct(self))->rb_buffer;
}

typedef struct {
    sfSound* handle;
    const sfSoundBuffer* buffer;
} SoundSetBufferArgs;

/* sfSound_setBuffer() calls Sound::stop() when a buffer is already attached,
   which can block waiting on the audio thread; release the GVL for the same
   reason as Sound_free/SS_METHOD(stop). */
static void* Sound_set_buffer_without_gvl(void* raw) {
    SoundSetBufferArgs* args = raw;

    sfSound_setBuffer(args->handle, args->buffer);

    return NULL;
}

/* call-seq:
 *   buffer=(value) -> SoundBuffer
 *
 * Attaches a new SoundBuffer. If a buffer is already attached and the sound
 * is playing, it is stopped first, which may briefly block the calling
 * thread (see #stop).
 *
 * @return [SoundBuffer] +value+
 * @raise [ArgumentError] if +value+ is not a SoundBuffer
 */
static VALUE Sound_set_buffer(VALUE self, VALUE rb_buffer) {
    Sound* sound = (Sound*)Get_Sound_Struct(self);
    SoundSetBufferArgs args;

    if (!rb_obj_is_kind_of(rb_buffer, Get_Klass_SoundBuffer())) {
        raise_invalid_argument_class(Get_Klass_SoundBuffer());
    }

    args.handle = sound->source.handle;
    args.buffer = Get_SoundBuffer_Struct(rb_buffer);

    rb_thread_call_without_gvl(Sound_set_buffer_without_gvl, &args, RUBY_UBF_IO, NULL);
    sound->rb_buffer = rb_buffer;

    return rb_buffer;
}

#define SS_FN(name) sfSound_##name
#define SS_METHOD(name) Sound_##name
#include "audio/sound_source.inc"
#undef SS_FN
#undef SS_METHOD

/* Document-class: SFML::Sound
 * A sound playing directly from a SoundBuffer held fully in memory. Suited
 * to short effects; for long files prefer Music, which streams instead.
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
void Init_Sound(VALUE rb_mSFML) {
    rb_cSound = rb_define_class_under(rb_mSFML, "Sound", rb_cObject);

    rb_define_singleton_method(rb_cSound, "new", Sound_new, 1);

    rb_define_method(rb_cSound, "copy", Sound_copy, 0);
    rb_define_method(rb_cSound, "buffer", Sound_get_buffer, 0);
    rb_define_method(rb_cSound, "buffer=", Sound_set_buffer, 1);

    Sound_define_sound_source_methods(rb_cSound);
}

VALUE Get_Klass_Sound(void) {
    return rb_cSound;
}

void* Get_Sound_Struct(VALUE self) {
    Sound* ptr;
    TypedData_Get_Struct(self, Sound, &Sound_data_type, ptr);
    return ptr;
}

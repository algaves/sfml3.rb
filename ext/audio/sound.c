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
    if (sound->source.handle != NULL) {
        rb_thread_call_without_gvl(Sound_destroy_without_gvl, sound->source.handle, RUBY_UBF_IO,
                                   NULL);
    }

    effect_processor_release(sound->source.effect_slot);
    free(sound);
}

/* Deliberately no RUBY_TYPED_FREE_IMMEDIATELY: Sound_free releases the GVL for
   sfSound_destroy, and freeing during GC (which the flag requests) would let
   another thread allocate while GC is mid-cycle, which Ruby aborts on as
   "object allocation during garbage collection phase". Deferred finalization
   runs the same free with the GVL held and no GC in progress. */
static const rb_data_type_t Sound_data_type = {
    .wrap_struct_name = "SF::Audio::Sound",
    .function = {.dmark = Sound_mark, .dfree = Sound_free, .dsize = NULL}};

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

/* call-seq: initialize_copy(other) -> self
 *
 * Copy construction is not supported: a Sound wraps a native resource that
 * cannot be duplicated, so this always raises.
 *
 * @raise [TypeError] always
 */
static VALUE Sound_initialize_copy(VALUE self, VALUE other) {
    (void)other;
    rb_raise(rb_eTypeError, "can't copy a %s", rb_obj_classname(self));
}

static VALUE Sound_alloc(VALUE klass) {
    Sound* ptr = malloc(sizeof(Sound));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate sound");
    }

    ptr->source.handle = NULL;
    ptr->source.effect_slot = -1;
    ptr->rb_buffer = Qnil;

    return TypedData_Wrap_Struct(klass, &Sound_data_type, ptr);
}

/* call-seq:
 *   Sound.new(buffer) -> Sound
 *
 * Creates a sound that plays the given buffer.
 *
 * @return [Sound]
 * @raise [ArgumentError] if +buffer+ is not a SoundBuffer
 */
static VALUE Sound_initialize(VALUE self, VALUE rb_buffer) {
    Sound* sound;
    sfSound* handle;

    if (!rb_obj_is_kind_of(rb_buffer, Get_Klass_SoundBuffer())) {
        raise_invalid_argument_class(Get_Klass_SoundBuffer());
    }

    sound = (Sound*)Get_Sound_Struct(self);

    handle = sfSound_create(Get_SoundBuffer_Struct(rb_buffer));

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound");
    }

    sound->source.handle = handle;
    sound->rb_buffer = rb_buffer;

    return self;
}

/* call-seq: copy -> Sound
 *
 * Creates an independent copy of the sound that shares its buffer.
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
 * Returns the buffer currently attached to this sound.
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

/* Document-class: SF::Audio::Sound
 * A sound playing directly from a SoundBuffer held fully in memory. Suited
 * to short effects; for long files prefer Music, which streams instead.
 * Derives from SoundSource.
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
void Init_Sound(VALUE rb_mAudio) {
    rb_cSound = rb_define_class_under(rb_mAudio, "Sound", Get_Klass_SoundSource());

    rb_define_alloc_func(rb_cSound, Sound_alloc);
    rb_define_method(rb_cSound, "initialize", Sound_initialize, 1);
    rb_define_private_method(rb_cSound, "initialize_copy", Sound_initialize_copy, 1);

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

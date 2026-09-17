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

    effect_processor_release(sound->source.effect_slot);
    rb_thread_call_without_gvl(Sound_destroy_without_gvl, sound->source.handle, RUBY_UBF_IO, NULL);
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

static VALUE Sound_new(VALUE klass, VALUE rb_buffer) {
    if (!rb_obj_is_kind_of(rb_buffer, Get_Klass_SoundBuffer())) {
        raise_invalid_argument_class(Get_Klass_SoundBuffer());
    }

    return Sound_wrap(klass, sfSound_create(Get_SoundBuffer_Struct(rb_buffer)), rb_buffer);
}

static VALUE Sound_copy(VALUE self) {
    Sound* sound = (Sound*)Get_Sound_Struct(self);

    return Sound_wrap(Get_Klass_Sound(), sfSound_copy((const sfSound*)sound->source.handle),
                      sound->rb_buffer);
}

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

void Init_Sound(VALUE rb_module) {
    rb_cSound = rb_define_class_under(rb_module, "Sound", rb_cObject);

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

#include "audio/sound.h"

#include <ruby.h>
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

static void Sound_mark(void *ptr) {
    Sound *sound = ptr;

    rb_gc_mark(sound->rb_buffer);
}

static void Sound_free(void *ptr) {
    Sound *sound = ptr;

    effect_processor_release(sound->source.effect_slot);
    sfSound_destroy(sound->source.handle);
    free(sound);
}

static const rb_data_type_t Sound_data_type = {
    .wrap_struct_name = "SFML::Sound",
    .function = {.dmark = Sound_mark, .dfree = Sound_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE Sound_wrap(VALUE klass, sfSound *handle, VALUE rb_buffer) {
    Sound *ptr;

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
    Sound *sound = (Sound *) Get_Sound_Struct(self);

    return Sound_wrap(Get_Klass_Sound(),
                      sfSound_copy((const sfSound *) sound->source.handle), sound->rb_buffer);
}

static VALUE Sound_get_buffer(VALUE self) {
    return ((Sound *) Get_Sound_Struct(self))->rb_buffer;
}

static VALUE Sound_set_buffer(VALUE self, VALUE rb_buffer) {
    Sound *sound = (Sound *) Get_Sound_Struct(self);

    if (!rb_obj_is_kind_of(rb_buffer, Get_Klass_SoundBuffer())) {
        raise_invalid_argument_class(Get_Klass_SoundBuffer());
    }

    sfSound_setBuffer(sound->source.handle, Get_SoundBuffer_Struct(rb_buffer));
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

void *Get_Sound_Struct(VALUE self) {
    Sound *ptr;
    TypedData_Get_Struct(self, Sound, &Sound_data_type, ptr);
    return ptr;
}

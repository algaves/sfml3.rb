#include "audio/sound_buffer_recorder.h"

#include <ruby.h>
#include <stdlib.h>

#include "audio/audio_enums.h"
#include "audio/sound_buffer.h"
#include "core/macros.h"

typedef struct {
    sfSoundBufferRecorder *handle;
} SoundBufferRecorder;

static VALUE rb_cSoundBufferRecorder;

static void SoundBufferRecorder_free(void *ptr) {
    SoundBufferRecorder *recorder = ptr;

    if (recorder->handle != NULL) {
        sfSoundBufferRecorder_destroy(recorder->handle);
    }

    free(recorder);
}

static const rb_data_type_t SoundBufferRecorder_data_type = {
    .wrap_struct_name = "SFML::SoundBufferRecorder",
    .function = {.dmark = NULL, .dfree = SoundBufferRecorder_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE SoundBufferRecorder_wrap(VALUE klass, sfSoundBufferRecorder *handle) {
    SoundBufferRecorder *ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound buffer recorder (no capture device?)");
    }

    ptr = malloc(sizeof(SoundBufferRecorder));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &SoundBufferRecorder_data_type, ptr);
}

static VALUE SoundBufferRecorder_new(VALUE klass) {
    return SoundBufferRecorder_wrap(klass, sfSoundBufferRecorder_create());
}

static VALUE SoundBufferRecorder_start(VALUE self, VALUE rb_sample_rate) {
    return BOOL2RB(sfSoundBufferRecorder_start(Get_SoundBufferRecorder_Struct(self),
                                               (unsigned int) NUM2INT(rb_sample_rate)));
}

static VALUE SoundBufferRecorder_stop(VALUE self) {
    sfSoundBufferRecorder_stop(Get_SoundBufferRecorder_Struct(self));
    return self;
}

static VALUE SoundBufferRecorder_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundBufferRecorder_getSampleRate(Get_SoundBufferRecorder_Struct(self)));
}

static VALUE SoundBufferRecorder_buffer(VALUE self) {
    return sound_buffer_from_borrowed(sfSoundBufferRecorder_getBuffer(Get_SoundBufferRecorder_Struct(self)));
}

static VALUE SoundBufferRecorder_device(VALUE self) {
    return rb_str_new_cstr(sfSoundBufferRecorder_getDevice(Get_SoundBufferRecorder_Struct(self)));
}

static VALUE SoundBufferRecorder_set_device(VALUE self, VALUE rb_name) {
    return BOOL2RB(sfSoundBufferRecorder_setDevice(Get_SoundBufferRecorder_Struct(self),
                                                   StringValueCStr(rb_name)));
}

static VALUE SoundBufferRecorder_channel_count(VALUE self) {
    return UINT2NUM(sfSoundBufferRecorder_getChannelCount(Get_SoundBufferRecorder_Struct(self)));
}

static VALUE SoundBufferRecorder_set_channel_count(VALUE self, VALUE rb_count) {
    sfSoundBufferRecorder_setChannelCount(Get_SoundBufferRecorder_Struct(self),
                                          (unsigned int) NUM2INT(rb_count));
    return rb_count;
}

void Init_SoundBufferRecorder(VALUE rb_module) {
    rb_cSoundBufferRecorder = rb_define_class_under(rb_module, "SoundBufferRecorder", rb_cObject);

    rb_define_singleton_method(rb_cSoundBufferRecorder, "new", SoundBufferRecorder_new, 0);

    rb_define_method(rb_cSoundBufferRecorder, "start", SoundBufferRecorder_start, 1);
    rb_define_method(rb_cSoundBufferRecorder, "stop", SoundBufferRecorder_stop, 0);
    rb_define_method(rb_cSoundBufferRecorder, "sample_rate", SoundBufferRecorder_sample_rate, 0);
    rb_define_method(rb_cSoundBufferRecorder, "buffer", SoundBufferRecorder_buffer, 0);
    rb_define_method(rb_cSoundBufferRecorder, "device", SoundBufferRecorder_device, 0);
    rb_define_method(rb_cSoundBufferRecorder, "device=", SoundBufferRecorder_set_device, 1);
    rb_define_method(rb_cSoundBufferRecorder, "channel_count", SoundBufferRecorder_channel_count, 0);
    rb_define_method(rb_cSoundBufferRecorder, "channel_count=", SoundBufferRecorder_set_channel_count, 1);
}

VALUE Get_Klass_SoundBufferRecorder(void) {
    return rb_cSoundBufferRecorder;
}

void *Get_SoundBufferRecorder_Struct(VALUE self) {
    SoundBufferRecorder *ptr;
    TypedData_Get_Struct(self, SoundBufferRecorder, &SoundBufferRecorder_data_type, ptr);
    return ptr->handle;
}

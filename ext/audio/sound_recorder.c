#include "audio/sound_recorder.h"

#include <ruby.h>
#include <ruby/thread.h>
#include <stdint.h>
#include <stdlib.h>

#include "audio/audio_enums.h"
#include "core/macros.h"

/* A subclassable recorder. The capture callbacks arrive on SFML's audio
   thread, so each re-enters Ruby through rb_thread_call_with_gvl and treats an
   exception as "stop capturing" instead of unwinding into the audio engine.
   A subclass only has to implement #on_process; #on_start and #on_stop are
   optional and default to continuing. */
typedef struct {
    VALUE rb_self;
    sfSoundRecorder *handle;
} SoundRecorder;

static VALUE rb_cSoundRecorder;

static void SoundRecorder_mark(void *ptr) {
    SoundRecorder *recorder = ptr;

    rb_gc_mark(recorder->rb_self);
}

static void SoundRecorder_free(void *ptr) {
    SoundRecorder *recorder = ptr;

    if (recorder->handle != NULL) {
        sfSoundRecorder_destroy(recorder->handle);
    }

    free(recorder);
}

static const rb_data_type_t SoundRecorder_data_type = {
    .wrap_struct_name = "SFML::SoundRecorder",
    .function = {.dmark = SoundRecorder_mark, .dfree = SoundRecorder_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

typedef struct {
    VALUE self;
    ID method;
} InvokeContext;

static VALUE SoundRecorder_invoke_body(VALUE raw) {
    InvokeContext *ctx = (InvokeContext *) raw;

    return rb_funcall(ctx->self, ctx->method, 0);
}

/* Calls a zero-argument Ruby method, swallowing any exception it raises. */
static void SoundRecorder_invoke(VALUE self, ID method) {
    InvokeContext ctx = {.self = self, .method = method};
    int state = 0;

    rb_protect(SoundRecorder_invoke_body, (VALUE) &ctx, &state);

    if (state) {
        rb_set_errinfo(Qnil);
    }
}

static bool SoundRecorder_on_start(void *userData) {
    SoundRecorder *recorder = userData;

    if (rb_respond_to(recorder->rb_self, rb_intern("on_start"))) {
        SoundRecorder_invoke(recorder->rb_self, rb_intern("on_start"));
    }

    return true;
}

typedef struct {
    SoundRecorder *recorder;
    const int16_t *samples;
    size_t sample_count;
    int ok;
} ProcessContext;

static VALUE SoundRecorder_process_body(VALUE raw) {
    ProcessContext *ctx = (ProcessContext *) raw;
    VALUE rb_samples = rb_ary_new_capa((long) ctx->sample_count);
    VALUE result;
    size_t i;

    for (i = 0; i < ctx->sample_count; i++) {
        rb_ary_push(rb_samples, INT2NUM(ctx->samples[i]));
    }

    result = rb_funcall(ctx->recorder->rb_self, rb_intern("on_process"), 1, rb_samples);
    ctx->ok = RTEST(result) ? 1 : 0;

    return Qnil;
}

static void *SoundRecorder_process_gvl(void *raw) {
    int state = 0;

    rb_protect(SoundRecorder_process_body, (VALUE) raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        ((ProcessContext *) raw)->ok = 0;
    }

    return NULL;
}

static bool SoundRecorder_on_process(const int16_t *samples, size_t sample_count, void *userData) {
    SoundRecorder *recorder = userData;
    ProcessContext ctx = {
        .recorder = recorder, .samples = samples, .sample_count = sample_count, .ok = 0
    };

    rb_thread_call_with_gvl(SoundRecorder_process_gvl, &ctx);

    return ctx.ok ? true : false;
}

static void SoundRecorder_on_stop(void *userData) {
    SoundRecorder *recorder = userData;

    if (rb_respond_to(recorder->rb_self, rb_intern("on_stop"))) {
        SoundRecorder_invoke(recorder->rb_self, rb_intern("on_stop"));
    }
}

static VALUE SoundRecorder_new(VALUE klass) {
    SoundRecorder *ptr;
    sfSoundRecorder *handle;
    VALUE self;

    ptr = malloc(sizeof(SoundRecorder));
    ptr->rb_self = Qnil;
    ptr->handle = NULL;

    self = TypedData_Wrap_Struct(klass, &SoundRecorder_data_type, ptr);
    ptr->rb_self = self;

    if (!rb_respond_to(self, rb_intern("on_process"))) {
        /* Ruby already owns the wrapper; SoundRecorder_free handles the NULL
           handle, so let GC clean up after the raise. */
        rb_raise(rb_eNotImpError, "subclass must define #on_process");
    }

    handle = sfSoundRecorder_create(SoundRecorder_on_start, SoundRecorder_on_process,
                                    SoundRecorder_on_stop, ptr);

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound recorder (no capture device?)");
    }

    ptr->handle = handle;

    return self;
}

static VALUE SoundRecorder_start(VALUE self, VALUE rb_sample_rate) {
    return BOOL2RB(sfSoundRecorder_start(Get_SoundRecorder_Struct(self),
                                         (unsigned int) NUM2INT(rb_sample_rate)));
}

static VALUE SoundRecorder_stop(VALUE self) {
    sfSoundRecorder_stop(Get_SoundRecorder_Struct(self));
    return self;
}

static VALUE SoundRecorder_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundRecorder_getSampleRate(Get_SoundRecorder_Struct(self)));
}

static VALUE SoundRecorder_available(VALUE klass) {
    return BOOL2RB(sfSoundRecorder_isAvailable());
}

static VALUE SoundRecorder_available_devices(VALUE klass) {
    size_t count = 0;
    const char *const *devices = sfSoundRecorder_getAvailableDevices(&count);
    VALUE rb_array;
    size_t i;

    if (devices == NULL) {
        return rb_ary_new();
    }

    rb_array = rb_ary_new_capa((long) count);

    for (i = 0; i < count; i++) {
        rb_ary_push(rb_array, rb_str_new_cstr(devices[i]));
    }

    return rb_array;
}

static VALUE SoundRecorder_default_device(VALUE klass) {
    return rb_str_new_cstr(sfSoundRecorder_getDefaultDevice());
}

static VALUE SoundRecorder_device(VALUE self) {
    return rb_str_new_cstr(sfSoundRecorder_getDevice(Get_SoundRecorder_Struct(self)));
}

static VALUE SoundRecorder_set_device(VALUE self, VALUE rb_name) {
    return BOOL2RB(
        sfSoundRecorder_setDevice(Get_SoundRecorder_Struct(self), StringValueCStr(rb_name)));
}

static VALUE SoundRecorder_channel_count(VALUE self) {
    return UINT2NUM(sfSoundRecorder_getChannelCount(Get_SoundRecorder_Struct(self)));
}

static VALUE SoundRecorder_set_channel_count(VALUE self, VALUE rb_count) {
    sfSoundRecorder_setChannelCount(Get_SoundRecorder_Struct(self), (unsigned int) NUM2INT(rb_count));
    return rb_count;
}

static VALUE SoundRecorder_channel_map(VALUE self) {
    size_t count = 0;
    sfSoundChannel *map = sfSoundRecorder_getChannelMap(Get_SoundRecorder_Struct(self), &count);
    VALUE rb_array;
    size_t i;

    if (map == NULL) {
        return rb_ary_new();
    }

    rb_array = rb_ary_new_capa((long) count);

    for (i = 0; i < count; i++) {
        rb_ary_push(rb_array, ID2SYM(rb_intern(sound_channel_name(map[i]))));
    }

    return rb_array;
}

void Init_SoundRecorder(VALUE rb_module) {
    rb_cSoundRecorder = rb_define_class_under(rb_module, "SoundRecorder", rb_cObject);

    rb_define_singleton_method(rb_cSoundRecorder, "new", SoundRecorder_new, 0);
    rb_define_singleton_method(rb_cSoundRecorder, "available?", SoundRecorder_available, 0);
    rb_define_singleton_method(rb_cSoundRecorder, "available_devices",
                               SoundRecorder_available_devices, 0);
    rb_define_singleton_method(rb_cSoundRecorder, "default_device", SoundRecorder_default_device, 0);

    rb_define_method(rb_cSoundRecorder, "start", SoundRecorder_start, 1);
    rb_define_method(rb_cSoundRecorder, "stop", SoundRecorder_stop, 0);
    rb_define_method(rb_cSoundRecorder, "sample_rate", SoundRecorder_sample_rate, 0);
    rb_define_method(rb_cSoundRecorder, "device", SoundRecorder_device, 0);
    rb_define_method(rb_cSoundRecorder, "device=", SoundRecorder_set_device, 1);
    rb_define_method(rb_cSoundRecorder, "channel_count", SoundRecorder_channel_count, 0);
    rb_define_method(rb_cSoundRecorder, "channel_count=", SoundRecorder_set_channel_count, 1);
    rb_define_method(rb_cSoundRecorder, "channel_map", SoundRecorder_channel_map, 0);
}

VALUE Get_Klass_SoundRecorder(void) {
    return rb_cSoundRecorder;
}

void *Get_SoundRecorder_Struct(VALUE self) {
    SoundRecorder *ptr;
    TypedData_Get_Struct(self, SoundRecorder, &SoundRecorder_data_type, ptr);
    return ptr->handle;
}

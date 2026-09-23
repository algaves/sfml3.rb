#include "audio/sound_recorder.h"

#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>

#include "audio/audio_enums.h"
#include "core/foreign_thread.h"
#include "core/macros.h"

/* A subclassable recorder. The capture callbacks arrive on SFML's capture
   thread -- a real OS thread never created by Ruby, exactly like the audio
   playback thread effect_processor.c/sound_stream.c are built to handle --
   so re-entering Ruby directly (e.g. via rb_thread_call_with_gvl, as this
   file previously did for #on_process, or via rb_protect with no GVL
   handling at all, as it previously did for #on_start/#on_stop) is a fatal
   VM error. Every callback here instead posts its work to the shared
   foreign-thread worker (core/foreign_thread.h), the same mechanism the
   other two files use, and treats an exception as "stop capturing" rather
   than unwinding into the audio engine. A subclass only has to implement
   #on_process; #on_start and #on_stop are optional and default to
   continuing. */

#define SOUND_RECORDER_JOB_TIMEOUT_MS 50
typedef struct {
    VALUE rb_self;
    sfSoundRecorder* handle;
} SoundRecorder;

static VALUE rb_cSoundRecorder;

/* CSFML reports "no such device" as NULL; that is an absent name, not an
   error, so it maps to nil rather than raising out of rb_str_new_cstr. */
static VALUE SoundRecorder_nullable_string(const char* value) {
    return value == NULL ? Qnil : rb_str_new_cstr(value);
}

static void SoundRecorder_mark(void* ptr) {
    SoundRecorder* recorder = ptr;

    rb_gc_mark(recorder->rb_self);
}

static void SoundRecorder_free(void* ptr) {
    SoundRecorder* recorder = ptr;

    if (recorder->handle != NULL) {
        sfSoundRecorder_destroy(recorder->handle);
    }

    free(recorder);
}

static const rb_data_type_t SoundRecorder_data_type = {
    .wrap_struct_name = "SF::Audio::SoundRecorder",
    .function = {.dmark = SoundRecorder_mark, .dfree = SoundRecorder_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

typedef struct {
    VALUE self;
    ID method;
} InvokeContext;

static VALUE SoundRecorder_invoke_body(VALUE raw) {
    InvokeContext* ctx = (InvokeContext*)raw;

    return rb_funcall(ctx->self, ctx->method, 0);
}

/* Runs on the shared foreign-thread worker, which holds the GVL. */
static void SoundRecorder_invoke_run(void* raw) {
    int state = 0;

    rb_protect(SoundRecorder_invoke_body, (VALUE)raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
    }
}

/* Calls a zero-argument Ruby method, swallowing any exception it raises. */
static void SoundRecorder_invoke(VALUE self, ID method) {
    InvokeContext ctx = {.self = self, .method = method};

    run_on_ruby_thread(SoundRecorder_invoke_run, &ctx, SOUND_RECORDER_JOB_TIMEOUT_MS);
}

static bool SoundRecorder_on_start(void* userData) {
    SoundRecorder* recorder = userData;

    if (rb_respond_to(recorder->rb_self, rb_intern("on_start"))) {
        SoundRecorder_invoke(recorder->rb_self, rb_intern("on_start"));
    }

    return true;
}

typedef struct {
    SoundRecorder* recorder;
    const int16_t* samples;
    size_t sample_count;
    int ok;
} ProcessContext;

static VALUE SoundRecorder_process_body(VALUE raw) {
    ProcessContext* ctx = (ProcessContext*)raw;
    VALUE rb_samples = rb_ary_new_capa((long)ctx->sample_count);
    VALUE result;
    size_t i;

    for (i = 0; i < ctx->sample_count; i++) {
        rb_ary_push(rb_samples, INT2NUM(ctx->samples[i]));
    }

    result = rb_funcall(ctx->recorder->rb_self, rb_intern("on_process"), 1, rb_samples);
    ctx->ok = RTEST(result) ? 1 : 0;

    return Qnil;
}

/* Runs on the shared foreign-thread worker, which holds the GVL. */
static void SoundRecorder_process_run(void* raw) {
    int state = 0;

    rb_protect(SoundRecorder_process_body, (VALUE)raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        ((ProcessContext*)raw)->ok = 0;
    }
}

static bool SoundRecorder_on_process(const int16_t* samples, size_t sample_count, void* userData) {
    SoundRecorder* recorder = userData;
    ProcessContext ctx = {
        .recorder = recorder, .samples = samples, .sample_count = sample_count, .ok = 0};

    if (!run_on_ruby_thread(SoundRecorder_process_run, &ctx, SOUND_RECORDER_JOB_TIMEOUT_MS)) {
        /* Timed out before the worker even claimed the job (e.g. GVL
           contention): drop this one chunk rather than block the capture
           thread indefinitely, but keep recording rather than aborting the
           whole session over one missed callback. */
        return true;
    }

    return ctx.ok ? true : false;
}

static void SoundRecorder_on_stop(void* userData) {
    SoundRecorder* recorder = userData;

    if (rb_respond_to(recorder->rb_self, rb_intern("on_stop"))) {
        SoundRecorder_invoke(recorder->rb_self, rb_intern("on_stop"));
    }
}

static VALUE SoundRecorder_alloc(VALUE klass) {
    SoundRecorder* ptr = malloc(sizeof(SoundRecorder));
    VALUE self;

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate sound recorder");
    }

    ptr->rb_self = Qnil;
    ptr->handle = NULL;

    self = TypedData_Wrap_Struct(klass, &SoundRecorder_data_type, ptr);
    ptr->rb_self = self;

    return self;
}

/* call-seq:
 *   SoundRecorder.new -> SoundRecorder
 *
 * SoundRecorder must be subclassed: the subclass is required to implement
 * +#on_process(samples)+, called from the audio thread with an Array of
 * captured Integer samples whenever a chunk is ready (return a truthy value
 * to keep recording, falsy to stop), and may optionally implement
 * +#on_start+ and +#on_stop+.
 *
 * @return [SoundRecorder]
 * @raise [NotImplementedError] if the subclass does not define #on_process
 * @raise [RuntimeError] if no capture device is available
 */
static VALUE SoundRecorder_initialize(VALUE self) {
    SoundRecorder* ptr;
    sfSoundRecorder* handle;

    TypedData_Get_Struct(self, SoundRecorder, &SoundRecorder_data_type, ptr);

    if (!rb_respond_to(self, rb_intern("on_process"))) {
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

/* call-seq:
 *   start(sample_rate) -> true or false
 *
 * Starts capturing audio at the given sample rate.
 *
 * @return [Boolean] whether recording started successfully
 */
static VALUE SoundRecorder_start(VALUE self, VALUE rb_sample_rate) {
    return BOOL2RB(sfSoundRecorder_start(Get_SoundRecorder_Struct(self),
                                         (unsigned int)NUM2INT(rb_sample_rate)));
}

/* call-seq: stop -> self
 *
 * Stops capturing audio.
 *
 * @return [self]
 */
static VALUE SoundRecorder_stop(VALUE self) {
    sfSoundRecorder_stop(Get_SoundRecorder_Struct(self));
    return self;
}

/* call-seq: sample_rate -> Integer
 *
 * Returns the sample rate used for capture.
 *
 * @return [Integer]
 */
static VALUE SoundRecorder_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundRecorder_getSampleRate(Get_SoundRecorder_Struct(self)));
}

/* call-seq: SoundRecorder.available? -> true or false
 *
 * Returns +true+ if the audio backend supports capture.
 *
 * @return [Boolean] whether the audio backend supports capture at all
 */
static VALUE SoundRecorder_available(VALUE klass) {
    return BOOL2RB(sfSoundRecorder_isAvailable());
}

/* call-seq: SoundRecorder.available_devices -> Array<String>
 *
 * Returns the names of the capture devices available on this system.
 *
 * @return [Array<String>] names of the capture devices available on this
 *   system
 */
static VALUE SoundRecorder_available_devices(VALUE klass) {
    size_t count = 0;
    const char* const* devices = sfSoundRecorder_getAvailableDevices(&count);
    VALUE rb_array;
    size_t i;

    if (devices == NULL) {
        return rb_ary_new();
    }

    rb_array = rb_ary_new_capa((long)count);

    for (i = 0; i < count; i++) {
        rb_ary_push(rb_array, rb_str_new_cstr(devices[i]));
    }

    return rb_array;
}

/* call-seq: SoundRecorder.default_device -> String or nil
 *
 * Returns the name of the system's default capture device.
 *
 * @return [String, nil] the name of the system's default capture device, or
 *   +nil+ when the system reports none
 */
static VALUE SoundRecorder_default_device(VALUE klass) {
    return SoundRecorder_nullable_string(sfSoundRecorder_getDefaultDevice());
}

/* call-seq: device -> String or nil
 *
 * Returns the name of the capture device in use.
 *
 * @return [String, nil] the name of the capture device in use, or +nil+ when
 *   the recorder has none
 */
static VALUE SoundRecorder_device(VALUE self) {
    return SoundRecorder_nullable_string(sfSoundRecorder_getDevice(Get_SoundRecorder_Struct(self)));
}

/* call-seq:
 *   device=(value) -> true or false
 *
 * Must be called while not recording. Get available names from
 * .available_devices.
 *
 * @return [Boolean] whether the device was set successfully
 */
static VALUE SoundRecorder_set_device(VALUE self, VALUE rb_name) {
    return BOOL2RB(
        sfSoundRecorder_setDevice(Get_SoundRecorder_Struct(self), StringValueCStr(rb_name)));
}

/* call-seq: channel_count -> Integer
 *
 * Returns the number of capture channels.
 *
 * @return [Integer]
 */
static VALUE SoundRecorder_channel_count(VALUE self) {
    return UINT2NUM(sfSoundRecorder_getChannelCount(Get_SoundRecorder_Struct(self)));
}

/* call-seq:
 *   channel_count=(value) -> Integer
 *
 * Must be called while not recording.
 *
 * @return [Integer] +value+
 */
static VALUE SoundRecorder_set_channel_count(VALUE self, VALUE rb_count) {
    sfSoundRecorder_setChannelCount(Get_SoundRecorder_Struct(self),
                                    (unsigned int)NUM2INT(rb_count));
    return rb_count;
}

/* call-seq: channel_map -> Array<Symbol>
 *
 * Returns the channel layout of the recorder.
 *
 * @return [Array<Symbol>] one entry per channel, e.g.
 *   +[:front_left, :front_right]+
 */
static VALUE SoundRecorder_channel_map(VALUE self) {
    size_t count = 0;
    sfSoundChannel* map = sfSoundRecorder_getChannelMap(Get_SoundRecorder_Struct(self), &count);
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

/* Document-class: SF::Audio::SoundRecorder
 * Base class for a custom audio capture consumer. Subclasses must implement
 * +#on_process(samples)+, called from the audio thread whenever a chunk of
 * captured samples is ready, and may implement +#on_start+/+#on_stop+ to
 * react to recording starting/stopping. For simply capturing into a
 * SoundBuffer without custom processing, use SoundBufferRecorder instead.
 */
void Init_SoundRecorder(VALUE rb_mAudio) {
    rb_cSoundRecorder = rb_define_class_under(rb_mAudio, "SoundRecorder", rb_cObject);

    rb_define_alloc_func(rb_cSoundRecorder, SoundRecorder_alloc);
    rb_define_method(rb_cSoundRecorder, "initialize", SoundRecorder_initialize, 0);
    rb_define_singleton_method(rb_cSoundRecorder, "available?", SoundRecorder_available, 0);
    rb_define_singleton_method(rb_cSoundRecorder, "available_devices",
                               SoundRecorder_available_devices, 0);
    rb_define_singleton_method(rb_cSoundRecorder, "default_device", SoundRecorder_default_device,
                               0);

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

void* Get_SoundRecorder_Struct(VALUE self) {
    SoundRecorder* ptr;
    TypedData_Get_Struct(self, SoundRecorder, &SoundRecorder_data_type, ptr);
    return ptr->handle;
}

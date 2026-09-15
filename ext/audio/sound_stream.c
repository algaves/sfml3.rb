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
#include "system/time.h"

/* A subclassable stream. SFML calls the two C callbacks from its audio thread,
   which does not hold the GVL, so each callback re-enters Ruby through
   rb_thread_call_with_gvl and reports an exception as "stop the stream" rather
   than unwinding into the audio engine. The sample buffer lives in the wrapper
   and is reused across callbacks, because SFML keeps the pointer it is handed
   until the next onGetData. */
typedef struct {
    SoundSource source;
    VALUE rb_self;
    int16_t *samples;
    size_t samples_capacity;
} SoundStream;

static VALUE rb_cSoundStream;

static void SoundStream_mark(void *ptr) {
    SoundStream *stream = ptr;

    rb_gc_mark(stream->rb_self);
}

static void SoundStream_free(void *ptr) {
    SoundStream *stream = ptr;

    effect_processor_release(stream->source.effect_slot);
    sfSoundStream_destroy(stream->source.handle);
    free(stream->samples);
    free(stream);
}

static const rb_data_type_t SoundStream_data_type = {
    .wrap_struct_name = "SFML::SoundStream",
    .function = {.dmark = SoundStream_mark, .dfree = SoundStream_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

/* Expands the stream's sample buffer if needed, then copies either a packed
   String of int16 or an Array of Integers into it. Returns the sample count. */
static size_t SoundStream_store_samples(SoundStream *stream, VALUE rb_samples) {
    size_t count;

    if (RB_TYPE_P(rb_samples, T_STRING)) {
        size_t bytes = (size_t) RSTRING_LEN(rb_samples);

        count = bytes / sizeof(int16_t);
    } else if (RB_TYPE_P(rb_samples, T_ARRAY)) {
        count = (size_t) RARRAY_LEN(rb_samples);
    } else {
        rb_raise(rb_eTypeError, "on_get_data must return an Array of samples, a packed String, or nil");
        return 0;
    }

    if (count > stream->samples_capacity) {
        int16_t *grown = realloc(stream->samples, count * sizeof(int16_t));

        if (grown == NULL) {
            rb_raise(rb_eNoMemError, "could not grow sound stream buffer");
        }

        stream->samples = grown;
        stream->samples_capacity = count;
    }

    if (RB_TYPE_P(rb_samples, T_STRING)) {
        memcpy(stream->samples, RSTRING_PTR(rb_samples), count * sizeof(int16_t));
    } else {
        size_t i;

        for (i = 0; i < count; i++) {
            stream->samples[i] = (int16_t) NUM2INT(rb_ary_entry(rb_samples, (long) i));
        }
    }

    return count;
}

typedef struct {
    SoundStream *stream;
    sfSoundStreamChunk *chunk;
    int ok;
} GetDataContext;

static VALUE SoundStream_get_data_body(VALUE raw) {
    GetDataContext *ctx = (GetDataContext *) raw;
    VALUE result = rb_funcall(ctx->stream->rb_self, rb_intern("on_get_data"), 0);

    if (NIL_P(result) || result == Qfalse) {
        ctx->ok = 0;
        return Qnil;
    }

    ctx->chunk->sampleCount = (unsigned int) SoundStream_store_samples(ctx->stream, result);
    ctx->chunk->samples = ctx->stream->samples;
    ctx->ok = 1;

    return Qnil;
}

static void *SoundStream_get_data_gvl(void *raw) {
    int state = 0;

    rb_protect(SoundStream_get_data_body, (VALUE) raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        ((GetDataContext *) raw)->ok = 0;
    }

    return NULL;
}

static bool SoundStream_on_get_data(sfSoundStreamChunk *chunk, void *userData) {
    SoundStream *stream = userData;
    GetDataContext ctx = {.stream = stream, .chunk = chunk, .ok = 0};

    rb_thread_call_with_gvl(SoundStream_get_data_gvl, &ctx);

    return ctx.ok ? true : false;
}

typedef struct {
    SoundStream *stream;
    sfTime time;
} SeekContext;

static VALUE SoundStream_seek_body(VALUE raw) {
    SeekContext *ctx = (SeekContext *) raw;

    rb_funcall(ctx->stream->rb_self, rb_intern("on_seek"), 1, time_to_rb(ctx->time));

    return Qnil;
}

static void *SoundStream_seek_gvl(void *raw) {
    int state = 0;

    rb_protect(SoundStream_seek_body, (VALUE) raw, &state);

    if (state) {
        rb_set_errinfo(Qnil);
    }

    return NULL;
}

static void SoundStream_on_seek(sfTime time, void *userData) {
    SoundStream *stream = userData;
    SeekContext ctx = {.stream = stream, .time = time};

    if (!rb_respond_to(stream->rb_self, rb_intern("on_seek"))) {
        return;
    }

    rb_thread_call_with_gvl(SoundStream_seek_gvl, &ctx);
}

static VALUE SoundStream_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_channel_count, rb_sample_rate, rb_channel_map;
    sfSoundChannel *channel_map = NULL;
    size_t channel_map_size = 0;
    SoundStream *ptr;
    VALUE self;
    long i;

    rb_scan_args(argc, argv, "21", &rb_channel_count, &rb_sample_rate, &rb_channel_map);

    if (!NIL_P(rb_channel_map)) {
        if (!RB_TYPE_P(rb_channel_map, T_ARRAY)) {
            rb_raise(rb_eArgError, "channel map must be an Array");
        }

        channel_map_size = (size_t) RARRAY_LEN(rb_channel_map);
        channel_map = malloc(sizeof(sfSoundChannel) * channel_map_size);

        for (i = 0; i < (long) channel_map_size; i++) {
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

    ptr->source.handle = sfSoundStream_create(SoundStream_on_get_data, SoundStream_on_seek,
                                              (unsigned int) NUM2INT(rb_channel_count),
                                              (unsigned int) NUM2INT(rb_sample_rate), channel_map,
                                              channel_map_size, ptr);

    free(channel_map);

    if (ptr->source.handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound stream");
    }

    return self;
}

static VALUE SoundStream_channel_count(VALUE self) {
    return UINT2NUM(sfSoundStream_getChannelCount(Get_SoundStream_Struct(self)));
}

static VALUE SoundStream_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundStream_getSampleRate(Get_SoundStream_Struct(self)));
}

static VALUE SoundStream_channel_map(VALUE self) {
    size_t count = 0;
    sfSoundChannel *map = sfSoundStream_getChannelMap(Get_SoundStream_Struct(self), &count);
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

#define SS_FN(name) sfSoundStream_##name
#define SS_METHOD(name) SoundStream_##name
#include "audio/sound_source.inc"
#undef SS_FN
#undef SS_METHOD

void Init_SoundStream(VALUE rb_module) {
    rb_cSoundStream = rb_define_class_under(rb_module, "SoundStream", rb_cObject);

    rb_define_singleton_method(rb_cSoundStream, "new", SoundStream_new, -1);

    rb_define_method(rb_cSoundStream, "channel_count", SoundStream_channel_count, 0);
    rb_define_method(rb_cSoundStream, "sample_rate", SoundStream_sample_rate, 0);
    rb_define_method(rb_cSoundStream, "channel_map", SoundStream_channel_map, 0);

    SoundStream_define_sound_source_methods(rb_cSoundStream);
}

VALUE Get_Klass_SoundStream(void) {
    return rb_cSoundStream;
}

void *Get_SoundStream_Struct(VALUE self) {
    SoundStream *ptr;
    TypedData_Get_Struct(self, SoundStream, &SoundStream_data_type, ptr);
    return ptr->source.handle;
}

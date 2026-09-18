#include "audio/sound_buffer.h"

#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "audio/audio_enums.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "system/input_stream.h"
#include "system/time.h"

typedef struct {
    sfSoundBuffer* buffer;
    bool owns;
} SoundBuffer;

static VALUE rb_cSoundBuffer;

static void SoundBuffer_free(void* ptr) {
    SoundBuffer* sound_buffer = ptr;

    if (sound_buffer->owns) {
        sfSoundBuffer_destroy(sound_buffer->buffer);
    }

    free(sound_buffer);
}

static const rb_data_type_t SoundBuffer_data_type = {
    .wrap_struct_name = "SFML::SoundBuffer",
    .function = {.dmark = NULL, .dfree = SoundBuffer_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE SoundBuffer_wrap(VALUE klass, sfSoundBuffer* buffer) {
    SoundBuffer* ptr;

    if (buffer == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sound buffer");
    }

    ptr = malloc(sizeof(SoundBuffer));
    ptr->buffer = buffer;
    ptr->owns = true;

    return TypedData_Wrap_Struct(klass, &SoundBuffer_data_type, ptr);
}

VALUE sound_buffer_from_borrowed(const sfSoundBuffer* buffer) {
    SoundBuffer* ptr;

    if (buffer == NULL) {
        return Qnil;
    }

    ptr = malloc(sizeof(SoundBuffer));
    ptr->buffer = (sfSoundBuffer*)buffer;
    ptr->owns = false;

    return TypedData_Wrap_Struct(rb_cSoundBuffer, &SoundBuffer_data_type, ptr);
}

/* call-seq:
 *   SoundBuffer.from_file(path) -> SoundBuffer
 *
 * @return [SoundBuffer]
 * @raise [RuntimeError] if the file cannot be opened or decoded
 */
static VALUE SoundBuffer_from_file(VALUE klass, VALUE rb_path) {
    return SoundBuffer_wrap(klass, sfSoundBuffer_createFromFile(StringValueCStr(rb_path)));
}

/* call-seq:
 *   SoundBuffer.from_memory(data) -> SoundBuffer
 *
 * @return [SoundBuffer]
 * @raise [RuntimeError] if +data+ cannot be decoded
 */
static VALUE SoundBuffer_from_memory(VALUE klass, VALUE rb_data) {
    StringValue(rb_data);

    return SoundBuffer_wrap(
        klass, sfSoundBuffer_createFromMemory(RSTRING_PTR(rb_data), (size_t)RSTRING_LEN(rb_data)));
}

/* call-seq:
 *   SoundBuffer.from_stream(stream) -> SoundBuffer
 *
 * @return [SoundBuffer]
 * @raise [RuntimeError] if the stream cannot be decoded
 */
static VALUE SoundBuffer_from_stream(VALUE klass, VALUE rb_stream) {
    VALUE holder = Qnil;
    sfInputStream* stream = input_stream_from_rb(rb_stream, &holder);

    (void)holder;

    return SoundBuffer_wrap(klass, sfSoundBuffer_createFromStream(stream));
}

/* SFML rejects a buffer whose channel map does not have exactly one entry per
   channel, so an omitted map is filled with the conventional layout for 1 and
   2 channels and left Unspecified beyond that. */
static void SoundBuffer_default_channel_map(unsigned int channel_count, sfSoundChannel* map) {
    static const sfSoundChannel surround[] = {
        sfSoundChannelFrontLeft,   sfSoundChannelFrontRight,
        sfSoundChannelFrontCenter, sfSoundChannelLowFrequencyEffects,
        sfSoundChannelBackLeft,    sfSoundChannelBackRight,
        sfSoundChannelSideLeft,    sfSoundChannelSideRight,
        sfSoundChannelTopCenter};
    unsigned int i;

    for (i = 0; i < channel_count; i++) {
        if (i < sizeof(surround) / sizeof(surround[0])) {
            map[i] = surround[i];
        } else {
            map[i] = sfSoundChannelUnspecified;
        }
    }

    if (channel_count == 1) {
        map[0] = sfSoundChannelMono;
    }
}

/* Copies the samples into a contiguous int16 buffer: an Array is converted
   element by element, and a String's packed int16 samples are copied out of
   the (possibly unaligned) Ruby storage. The channel map, when given, is
   translated symbol by symbol. */
/* call-seq:
 *   SoundBuffer.from_samples(samples, channel_count, sample_rate)              -> SoundBuffer
 *   SoundBuffer.from_samples(samples, channel_count, sample_rate, channel_map) -> SoundBuffer
 *
 * +samples+ is an Array of Integer samples or a packed String of int16
 * samples, interleaved by channel. +channel_map+, if given, is an Array of
 * channel Symbols (see SoundChannel), one per channel; if omitted, a
 * conventional layout is assumed for 1 and 2 channels.
 *
 * @return [SoundBuffer]
 * @raise [ArgumentError] if +channel_count+ is 0, +samples+ is neither an
 *   Array nor a String, or +channel_map+ doesn't have exactly one entry per
 *   channel
 */
static VALUE SoundBuffer_from_samples(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_samples, rb_channel_count, rb_sample_rate, rb_channel_map;
    int16_t* samples;
    sfSoundChannel* channel_map;
    unsigned int channel_count;
    uint64_t sample_count;
    sfSoundBuffer* buffer;

    rb_scan_args(argc, argv, "31", &rb_samples, &rb_channel_count, &rb_sample_rate,
                 &rb_channel_map);

    channel_count = (unsigned int)NUM2INT(rb_channel_count);

    if (channel_count == 0) {
        rb_raise(rb_eArgError, "channel count must be positive");
    }

    if (RB_TYPE_P(rb_samples, T_STRING)) {
        size_t bytes = (size_t)RSTRING_LEN(rb_samples);

        if (bytes % sizeof(int16_t) != 0) {
            rb_raise(rb_eArgError, "sample string length must be a multiple of 2 bytes");
        }

        sample_count = bytes / sizeof(int16_t);
        samples = malloc(bytes > 0 ? bytes : 1);

        if (bytes > 0) {
            const int16_t* src = (const int16_t*)RSTRING_PTR(rb_samples);
            size_t i;

            for (i = 0; i < sample_count; i++) {
                samples[i] = src[i];
            }
        }
    } else if (RB_TYPE_P(rb_samples, T_ARRAY)) {
        long i;

        sample_count = (uint64_t)RARRAY_LEN(rb_samples);
        samples = malloc(sizeof(int16_t) * ((size_t)sample_count + 1));

        for (i = 0; i < (long)sample_count; i++) {
            samples[i] = (int16_t)NUM2INT(rb_ary_entry(rb_samples, i));
        }
    } else {
        rb_raise(rb_eArgError, "samples must be an Array of integers or a String");
        return Qnil;
    }

    channel_map = malloc(sizeof(sfSoundChannel) * channel_count);

    if (NIL_P(rb_channel_map)) {
        SoundBuffer_default_channel_map(channel_count, channel_map);
    } else {
        long i;

        if (!RB_TYPE_P(rb_channel_map, T_ARRAY) ||
            (unsigned long)RARRAY_LEN(rb_channel_map) != channel_count) {
            free(samples);
            free(channel_map);
            rb_raise(rb_eArgError, "channel map must have exactly one entry per channel");
        }

        for (i = 0; i < (long)channel_count; i++) {
            channel_map[i] = sound_channel_from_rb(rb_ary_entry(rb_channel_map, i));
        }
    }

    buffer = sfSoundBuffer_createFromSamples(samples, sample_count, channel_count,
                                             (unsigned int)NUM2INT(rb_sample_rate), channel_map,
                                             channel_count);

    free(samples);
    free(channel_map);

    return SoundBuffer_wrap(klass, buffer);
}

/* call-seq: copy -> SoundBuffer
 *
 * @return [SoundBuffer] an independent copy
 */
static VALUE SoundBuffer_copy(VALUE self) {
    return SoundBuffer_wrap(Get_Klass_SoundBuffer(),
                            sfSoundBuffer_copy(Get_SoundBuffer_Struct(self)));
}

/* call-seq:
 *   save_to_file(path) -> true or false
 *
 * @return [Boolean] whether the file was written successfully
 */
static VALUE SoundBuffer_save_to_file(VALUE self, VALUE rb_path) {
    return BOOL2RB(
        sfSoundBuffer_saveToFile(Get_SoundBuffer_Struct(self), StringValueCStr(rb_path)));
}

/* call-seq: samples -> Array<Integer>
 *
 * @return [Array<Integer>] the raw int16 audio samples, interleaved by
 *   channel
 */
static VALUE SoundBuffer_samples(VALUE self) {
    const int16_t* samples = sfSoundBuffer_getSamples(Get_SoundBuffer_Struct(self));
    uint64_t count = sfSoundBuffer_getSampleCount(Get_SoundBuffer_Struct(self));
    VALUE rb_array;
    uint64_t i;

    if (samples == NULL) {
        return rb_ary_new();
    }

    rb_array = rb_ary_new_capa((long)count);

    for (i = 0; i < count; i++) {
        rb_ary_push(rb_array, INT2NUM(samples[i]));
    }

    return rb_array;
}

/* call-seq: sample_count -> Integer
 *
 * @return [Integer] total number of int16 samples, across all channels
 */
static VALUE SoundBuffer_sample_count(VALUE self) {
    return ULL2NUM(sfSoundBuffer_getSampleCount(Get_SoundBuffer_Struct(self)));
}

/* call-seq: sample_rate -> Integer
 *
 * @return [Integer]
 */
static VALUE SoundBuffer_sample_rate(VALUE self) {
    return UINT2NUM(sfSoundBuffer_getSampleRate(Get_SoundBuffer_Struct(self)));
}

/* call-seq: channel_count -> Integer
 *
 * @return [Integer]
 */
static VALUE SoundBuffer_channel_count(VALUE self) {
    return UINT2NUM(sfSoundBuffer_getChannelCount(Get_SoundBuffer_Struct(self)));
}

/* call-seq: channel_map -> Array<Symbol>
 *
 * @return [Array<Symbol>] one entry per channel, e.g.
 *   +[:front_left, :front_right]+
 */
static VALUE SoundBuffer_channel_map(VALUE self) {
    size_t count = 0;
    sfSoundChannel* map = sfSoundBuffer_getChannelMap(Get_SoundBuffer_Struct(self), &count);
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

/* call-seq: duration -> Time
 *
 * @return [Time]
 */
static VALUE SoundBuffer_duration(VALUE self) {
    return time_to_rb(sfSoundBuffer_getDuration(Get_SoundBuffer_Struct(self)));
}

/* Document-class: SFML::SoundBuffer
 * Audio samples held fully decoded in memory, ready to be played through one
 * or more Sound instances (a single buffer may back several simultaneous
 * Sounds). For long audio, prefer Music, which streams instead of loading
 * everything up front.
 */
void Init_SoundBuffer(VALUE rb_mSFML) {
    rb_cSoundBuffer = rb_define_class_under(rb_mSFML, "SoundBuffer", rb_cObject);

    rb_define_singleton_method(rb_cSoundBuffer, "from_file", SoundBuffer_from_file, 1);
    rb_define_singleton_method(rb_cSoundBuffer, "from_memory", SoundBuffer_from_memory, 1);
    rb_define_singleton_method(rb_cSoundBuffer, "from_stream", SoundBuffer_from_stream, 1);
    rb_define_singleton_method(rb_cSoundBuffer, "from_samples", SoundBuffer_from_samples, -1);

    rb_define_method(rb_cSoundBuffer, "copy", SoundBuffer_copy, 0);
    rb_define_method(rb_cSoundBuffer, "save_to_file", SoundBuffer_save_to_file, 1);
    rb_define_method(rb_cSoundBuffer, "samples", SoundBuffer_samples, 0);
    rb_define_method(rb_cSoundBuffer, "sample_count", SoundBuffer_sample_count, 0);
    rb_define_method(rb_cSoundBuffer, "sample_rate", SoundBuffer_sample_rate, 0);
    rb_define_method(rb_cSoundBuffer, "channel_count", SoundBuffer_channel_count, 0);
    rb_define_method(rb_cSoundBuffer, "channel_map", SoundBuffer_channel_map, 0);
    rb_define_method(rb_cSoundBuffer, "duration", SoundBuffer_duration, 0);
}

VALUE Get_Klass_SoundBuffer(void) {
    return rb_cSoundBuffer;
}

void* Get_SoundBuffer_Struct(VALUE self) {
    SoundBuffer* ptr;
    TypedData_Get_Struct(self, SoundBuffer, &SoundBuffer_data_type, ptr);
    return ptr->buffer;
}

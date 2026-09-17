#include "audio/effect_processor.h"

#include <ruby.h>
#include <string.h>

#include "core/foreign_thread.h"

/* Slots are process-global because a C callback cannot carry context. Each
   entry is either Qnil (free) or the Proc assigned to that slot. */
static VALUE effect_processors[EFFECT_PROCESSOR_SLOTS];

typedef struct {
    int slot;
    const float* input;
    unsigned int input_count;
    float* output;
    unsigned int output_count;
    unsigned int requested_output_count;
    unsigned int channels;
} EffectContext;

/* Fills the output buffer with silence for the full frame count the audio
   engine asked for. Used whenever the processor cannot produce real output
   (an exception, or a timed-out worker): reporting output_count 0 instead of
   this was observed to make miniaudio's node graph re-invoke this callback
   in an immediate, unbounded retry loop (tens of thousands of calls/second)
   rather than treat it as one completed (silent) buffer - which in turn
   starved SFML's AudioDevice::waitForReadingComplete() on the main thread,
   since the audio thread never stopped re-acquiring its internal mutex. */
static void effect_output_silence(EffectContext* ctx) {
    unsigned int i;

    for (i = 0; i < ctx->requested_output_count * ctx->channels; i++) {
        ctx->output[i] = 0.0f;
    }

    ctx->output_count = ctx->requested_output_count;
}

#define EFFECT_JOB_TIMEOUT_MS 50

void Init_EffectProcessor(void) {
    int i;

    for (i = 0; i < EFFECT_PROCESSOR_SLOTS; i++) {
        effect_processors[i] = Qnil;
        rb_gc_register_address(&effect_processors[i]);
    }
}

static VALUE effect_yield(VALUE raw) {
    EffectContext* ctx = (EffectContext*)raw;
    VALUE rb_input;
    VALUE rb_result;
    unsigned int i;
    unsigned int c;
    long produced;

    rb_input = rb_ary_new_capa((long)ctx->input_count);

    for (i = 0; i < ctx->input_count; i++) {
        VALUE frame = rb_ary_new_capa((long)ctx->channels);

        for (c = 0; c < ctx->channels; c++) {
            rb_ary_push(frame, DBL2NUM((double)ctx->input[i * ctx->channels + c]));
        }

        rb_ary_push(rb_input, frame);
    }

    rb_result = rb_funcall(effect_processors[ctx->slot], rb_intern("call"), 2, rb_input,
                           UINT2NUM(ctx->channels));

    if (!RB_TYPE_P(rb_result, T_ARRAY)) {
        rb_raise(rb_eTypeError, "effect processor must return an Array of frames");
    }

    produced = RARRAY_LEN(rb_result);

    if (produced > (long)ctx->output_count) {
        produced = (long)ctx->output_count;
    }

    for (i = 0; i < (unsigned int)produced; i++) {
        VALUE frame = rb_ary_entry(rb_result, (long)i);

        if (!RB_TYPE_P(frame, T_ARRAY)) {
            rb_raise(rb_eTypeError, "effect processor frames must be Arrays");
        }

        for (c = 0; c < ctx->channels; c++) {
            ctx->output[i * ctx->channels + c] = (float)NUM2DBL(rb_ary_entry(frame, (long)c));
        }
    }

    ctx->output_count = (unsigned int)produced;

    return Qnil;
}

/* Runs on the shared foreign-thread worker, which holds the GVL. */
static void effect_run(void* raw) {
    EffectContext* ctx = (EffectContext*)raw;
    int state = 0;

    rb_protect(effect_yield, (VALUE)ctx, &state);

    if (state) {
        /* An exception must not unwind through the audio engine: drop the
           output and let playback continue silently. */
        rb_set_errinfo(Qnil);
        effect_output_silence(ctx);
    }
}

static void effect_invoke(int slot, const float* input, unsigned int* input_count, float* output,
                          unsigned int* output_count, unsigned int channels) {
    EffectContext ctx;

    if (slot < 0 || slot >= EFFECT_PROCESSOR_SLOTS || NIL_P(effect_processors[slot]) ||
        *output_count == 0) {
        *output_count = 0;
        return;
    }

    ctx.slot = slot;
    ctx.input = input;
    ctx.input_count = (input != NULL) ? *input_count : 0;
    ctx.output = output;
    ctx.output_count = *output_count;
    ctx.requested_output_count = *output_count;
    ctx.channels = channels;

    if (!run_on_ruby_thread(effect_run, &ctx, EFFECT_JOB_TIMEOUT_MS)) {
        /* Timed out before the worker even claimed the job (e.g. GVL
           contention): degrade to silence for this callback rather than
           stall the audio thread. */
        effect_output_silence(&ctx);
    }

    /* The processor consumed every input frame it was offered. */
    *input_count = ctx.input_count;
    *output_count = ctx.output_count;
}

#define EFFECT_THUNK(n)                                                                            \
    static void effect_thunk_##n(const float* input, unsigned int* input_count, float* output,     \
                                 unsigned int* output_count, unsigned int channels) {              \
        effect_invoke(n, input, input_count, output, output_count, channels);                      \
    }

EFFECT_THUNK(0)
EFFECT_THUNK(1)
EFFECT_THUNK(2)
EFFECT_THUNK(3)
EFFECT_THUNK(4)
EFFECT_THUNK(5)
EFFECT_THUNK(6)
EFFECT_THUNK(7)
EFFECT_THUNK(8)
EFFECT_THUNK(9)
EFFECT_THUNK(10)
EFFECT_THUNK(11)
EFFECT_THUNK(12)
EFFECT_THUNK(13)
EFFECT_THUNK(14)
EFFECT_THUNK(15)
EFFECT_THUNK(16)
EFFECT_THUNK(17)
EFFECT_THUNK(18)
EFFECT_THUNK(19)
EFFECT_THUNK(20)
EFFECT_THUNK(21)
EFFECT_THUNK(22)
EFFECT_THUNK(23)
EFFECT_THUNK(24)
EFFECT_THUNK(25)
EFFECT_THUNK(26)
EFFECT_THUNK(27)
EFFECT_THUNK(28)
EFFECT_THUNK(29)
EFFECT_THUNK(30)
EFFECT_THUNK(31)

static sfEffectProcessor effect_thunks[EFFECT_PROCESSOR_SLOTS] = {
    effect_thunk_0,  effect_thunk_1,  effect_thunk_2,  effect_thunk_3,  effect_thunk_4,
    effect_thunk_5,  effect_thunk_6,  effect_thunk_7,  effect_thunk_8,  effect_thunk_9,
    effect_thunk_10, effect_thunk_11, effect_thunk_12, effect_thunk_13, effect_thunk_14,
    effect_thunk_15, effect_thunk_16, effect_thunk_17, effect_thunk_18, effect_thunk_19,
    effect_thunk_20, effect_thunk_21, effect_thunk_22, effect_thunk_23, effect_thunk_24,
    effect_thunk_25, effect_thunk_26, effect_thunk_27, effect_thunk_28, effect_thunk_29,
    effect_thunk_30, effect_thunk_31};

sfEffectProcessor effect_processor_acquire(VALUE rb_proc, int* slot) {
    int i;

    /* Replacing the processor on a source that already has one reuses its
       slot, so a source never leaks slots across repeated assignment. */
    if (*slot >= 0 && *slot < EFFECT_PROCESSOR_SLOTS) {
        if (NIL_P(rb_proc)) {
            effect_processors[*slot] = Qnil;
            return NULL;
        }

        effect_processors[*slot] = rb_proc;

        return effect_thunks[*slot];
    }

    if (NIL_P(rb_proc)) {
        *slot = -1;
        return NULL;
    }

    for (i = 0; i < EFFECT_PROCESSOR_SLOTS; i++) {
        if (NIL_P(effect_processors[i])) {
            effect_processors[i] = rb_proc;
            *slot = i;

            return effect_thunks[i];
        }
    }

    rb_raise(rb_eRuntimeError, "all %d effect-processor slots are in use", EFFECT_PROCESSOR_SLOTS);

    return NULL;
}

void effect_processor_release(int slot) {
    if (slot >= 0 && slot < EFFECT_PROCESSOR_SLOTS) {
        effect_processors[slot] = Qnil;
    }
}

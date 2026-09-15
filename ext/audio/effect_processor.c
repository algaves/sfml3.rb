#include "audio/effect_processor.h"

#include <pthread.h>
#include <ruby.h>
#include <ruby/thread.h>
#include <string.h>
#include <time.h>

/* Slots are process-global because a C callback cannot carry context. Each
   entry is either Qnil (free) or the Proc assigned to that slot. */
static VALUE effect_processors[EFFECT_PROCESSOR_SLOTS];

typedef struct {
    int slot;
    const float *input;
    unsigned int input_count;
    float *output;
    unsigned int output_count;
    unsigned int channels;
} EffectContext;

/* SFML's audio thread is a foreign native thread that Ruby never created, so
   it cannot legally call back into Ruby (rb_thread_call_with_gvl() requires
   the calling thread to have previously released the GVL via
   rb_thread_call_without_gvl(), which only a genuine Ruby thread can do -
   calling it from a foreign thread is a fatal VM error). Instead, a single
   persistent Ruby-owned worker thread is started once and blocks (without
   the GVL) on this mutex/condvar pair; the audio thread hands it a job and
   waits (bounded, so a wedged Ruby side can never hang real-time audio)
   while the worker runs the actual Ruby call under the GVL it legitimately
   holds. */
static pthread_mutex_t effect_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t effect_cond = PTHREAD_COND_INITIALIZER;
static EffectContext *volatile effect_pending_ctx = NULL;
static volatile int effect_pending_done = 0;

#define EFFECT_JOB_TIMEOUT_MS 50

static VALUE effect_worker_main(void *unused);

void Init_EffectProcessor(void) {
    int i;

    for (i = 0; i < EFFECT_PROCESSOR_SLOTS; i++) {
        effect_processors[i] = Qnil;
        rb_gc_register_address(&effect_processors[i]);
    }

    rb_thread_create(effect_worker_main, NULL);
}

static VALUE effect_yield(VALUE raw) {
    EffectContext *ctx = (EffectContext *) raw;
    VALUE rb_input;
    VALUE rb_result;
    unsigned int i;
    unsigned int c;
    long produced;

    rb_input = rb_ary_new_capa((long) ctx->input_count);

    for (i = 0; i < ctx->input_count; i++) {
        VALUE frame = rb_ary_new_capa((long) ctx->channels);

        for (c = 0; c < ctx->channels; c++) {
            rb_ary_push(frame, DBL2NUM((double) ctx->input[i * ctx->channels + c]));
        }

        rb_ary_push(rb_input, frame);
    }

    rb_result = rb_funcall(effect_processors[ctx->slot], rb_intern("call"), 2, rb_input,
                           UINT2NUM(ctx->channels));

    if (!RB_TYPE_P(rb_result, T_ARRAY)) {
        rb_raise(rb_eTypeError, "effect processor must return an Array of frames");
    }

    produced = RARRAY_LEN(rb_result);

    if (produced > (long) ctx->output_count) {
        produced = (long) ctx->output_count;
    }

    for (i = 0; i < (unsigned int) produced; i++) {
        VALUE frame = rb_ary_entry(rb_result, (long) i);

        if (!RB_TYPE_P(frame, T_ARRAY)) {
            rb_raise(rb_eTypeError, "effect processor frames must be Arrays");
        }

        for (c = 0; c < ctx->channels; c++) {
            ctx->output[i * ctx->channels + c] = (float) NUM2DBL(rb_ary_entry(frame, (long) c));
        }
    }

    ctx->output_count = (unsigned int) produced;

    return Qnil;
}

/* Runs on the worker thread with the GVL released. Blocks until the audio
   thread hands off a job, or forever if none ever arrives - this is fine,
   since a background thread does not keep the process alive on its own. */
static void *effect_worker_wait(void *unused) {
    (void) unused;

    pthread_mutex_lock(&effect_mutex);
    while (effect_pending_ctx == NULL) {
        pthread_cond_wait(&effect_cond, &effect_mutex);
    }
    pthread_mutex_unlock(&effect_mutex);

    return NULL;
}

/* The Ruby thread body. Alternates between waiting without the GVL and
   running the assigned Proc with it, so it is always in a valid state to
   call into Ruby - unlike the foreign audio thread that hands it jobs. */
static VALUE effect_worker_main(void *unused) {
    (void) unused;

    for (;;) {
        EffectContext *ctx;
        int state = 0;

        rb_thread_call_without_gvl(effect_worker_wait, NULL, RUBY_UBF_IO, NULL);

        pthread_mutex_lock(&effect_mutex);
        ctx = effect_pending_ctx;
        pthread_mutex_unlock(&effect_mutex);

        if (ctx == NULL) {
            continue;
        }

        rb_protect(effect_yield, (VALUE) ctx, &state);

        if (state) {
            /* An exception must not unwind through the audio engine: drop
               the output and let playback continue silently. */
            rb_set_errinfo(Qnil);
            ctx->output_count = 0;
        }

        pthread_mutex_lock(&effect_mutex);
        effect_pending_ctx = NULL;
        effect_pending_done = 1;
        pthread_cond_broadcast(&effect_cond);
        pthread_mutex_unlock(&effect_mutex);
    }

    return Qnil;
}

/* Runs on SFML's audio thread, which does not hold the GVL and was never
   created by Ruby. Hands the job to the worker thread and waits - with a
   timeout, so a stuck or slow Ruby side degrades to silence instead of
   stalling the audio callback indefinitely. */
static void effect_dispatch(EffectContext *ctx) {
    struct timespec deadline;

    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_nsec += (long) EFFECT_JOB_TIMEOUT_MS * 1000000L;
    deadline.tv_sec += deadline.tv_nsec / 1000000000L;
    deadline.tv_nsec %= 1000000000L;

    pthread_mutex_lock(&effect_mutex);

    while (effect_pending_ctx != NULL) {
        /* Another job is still in flight (should not normally happen with a
           single audio thread, but guards against it regardless). */
        pthread_cond_wait(&effect_cond, &effect_mutex);
    }

    effect_pending_ctx = ctx;
    effect_pending_done = 0;
    pthread_cond_broadcast(&effect_cond);

    while (!effect_pending_done) {
        int rc = pthread_cond_timedwait(&effect_cond, &effect_mutex, &deadline);

        if (rc != 0 && !effect_pending_done) {
            /* Timed out: give up on this callback rather than block audio
               forever. The worker may still complete it later, at which
               point it will find effect_pending_ctx already cleared. */
            ctx->output_count = 0;
            effect_pending_ctx = NULL;
            effect_pending_done = 1;
            break;
        }
    }

    pthread_mutex_unlock(&effect_mutex);
}

static void effect_invoke(int slot, const float *input, unsigned int *input_count, float *output,
                          unsigned int *output_count, unsigned int channels) {
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
    ctx.channels = channels;

    effect_dispatch(&ctx);

    /* The processor consumed every input frame it was offered. */
    *input_count = ctx.input_count;
    *output_count = ctx.output_count;
}

#define EFFECT_THUNK(n)                                                                          \
    static void effect_thunk_##n(const float *input, unsigned int *input_count, float *output,   \
                                 unsigned int *output_count, unsigned int channels) {            \
        effect_invoke(n, input, input_count, output, output_count, channels);                    \
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
    effect_thunk_0,  effect_thunk_1,  effect_thunk_2,  effect_thunk_3,
    effect_thunk_4,  effect_thunk_5,  effect_thunk_6,  effect_thunk_7,
    effect_thunk_8,  effect_thunk_9,  effect_thunk_10, effect_thunk_11,
    effect_thunk_12, effect_thunk_13, effect_thunk_14, effect_thunk_15,
    effect_thunk_16, effect_thunk_17, effect_thunk_18, effect_thunk_19,
    effect_thunk_20, effect_thunk_21, effect_thunk_22, effect_thunk_23,
    effect_thunk_24, effect_thunk_25, effect_thunk_26, effect_thunk_27,
    effect_thunk_28, effect_thunk_29, effect_thunk_30, effect_thunk_31
};

sfEffectProcessor effect_processor_acquire(VALUE rb_proc, int *slot) {
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

    rb_raise(rb_eRuntimeError, "all %d effect-processor slots are in use",
             EFFECT_PROCESSOR_SLOTS);

    return NULL;
}

void effect_processor_release(int slot) {
    if (slot >= 0 && slot < EFFECT_PROCESSOR_SLOTS) {
        effect_processors[slot] = Qnil;
    }
}

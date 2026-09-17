#include "core/foreign_thread.h"

#include <pthread.h>
#include <ruby.h>
#include <ruby/thread.h>
#include <time.h>

typedef enum {
    FT_IDLE = 0,
    FT_POSTED,  /* job handed off, worker has not claimed it yet */
    FT_RUNNING, /* worker has claimed the job and is running fn(ctx) */
    FT_DONE
} ForeignThreadState;

typedef struct {
    void (*fn)(void*);
    void* ctx;
} ForeignJob;

static pthread_mutex_t ft_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ft_cond = PTHREAD_COND_INITIALIZER;
static ForeignJob* volatile ft_job = NULL;
static volatile ForeignThreadState ft_state = FT_IDLE;
static volatile int ft_shutdown = 0;

static VALUE ft_worker_main(void* unused);

void Init_ForeignThread(void) {
    rb_thread_create(ft_worker_main, NULL);
}

/* Runs without the GVL on the worker thread. Blocks until a job is posted or
   shutdown is requested. */
static void* ft_wait_for_post(void* unused) {
    (void)unused;

    pthread_mutex_lock(&ft_mutex);
    while (ft_state != FT_POSTED && !ft_shutdown) {
        pthread_cond_wait(&ft_cond, &ft_mutex);
    }
    pthread_mutex_unlock(&ft_mutex);

    return NULL;
}

/* Called by Ruby to interrupt ft_wait_for_post (e.g. at VM shutdown). */
static void ft_unblock(void* unused) {
    (void)unused;

    pthread_mutex_lock(&ft_mutex);
    ft_shutdown = 1;
    pthread_cond_broadcast(&ft_cond);
    pthread_mutex_unlock(&ft_mutex);
}

static VALUE ft_worker_main(void* unused) {
    (void)unused;

    for (;;) {
        ForeignJob* job;

        rb_thread_call_without_gvl(ft_wait_for_post, NULL, ft_unblock, NULL);

        pthread_mutex_lock(&ft_mutex);

        if (ft_shutdown) {
            pthread_mutex_unlock(&ft_mutex);
            break;
        }

        if (ft_state != FT_POSTED) {
            /* Spurious wake, or the caller already abandoned this job while
               it was still only posted - nothing to do. */
            pthread_mutex_unlock(&ft_mutex);
            continue;
        }

        job = ft_job;
        ft_state = FT_RUNNING;
        pthread_mutex_unlock(&ft_mutex);

        job->fn(job->ctx);

        pthread_mutex_lock(&ft_mutex);
        ft_state = FT_DONE;
        pthread_cond_broadcast(&ft_cond);
        pthread_mutex_unlock(&ft_mutex);
    }

    return Qnil;
}

int run_on_ruby_thread(void (*fn)(void* ctx), void* ctx, unsigned long timeout_ms) {
    ForeignJob job = {.fn = fn, .ctx = ctx};
    struct timespec deadline;
    int completed;
    int use_timeout = (timeout_ms > 0);

    if (use_timeout) {
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
        deadline.tv_sec += (time_t)(timeout_ms / 1000) + deadline.tv_nsec / 1000000000L;
        deadline.tv_nsec %= 1000000000L;
    }

    pthread_mutex_lock(&ft_mutex);

    while (ft_state != FT_IDLE) {
        pthread_cond_wait(&ft_cond, &ft_mutex);
    }

    ft_job = &job;
    ft_state = FT_POSTED;
    pthread_cond_broadcast(&ft_cond);

    while (ft_state != FT_DONE) {
        if (use_timeout) {
            int rc = pthread_cond_timedwait(&ft_cond, &ft_mutex, &deadline);

            if (rc != 0 && ft_state == FT_POSTED) {
                /* Still not claimed: safe to abandon, the worker has never
                   touched job/ctx. */
                ft_state = FT_IDLE;
                ft_job = NULL;
                pthread_cond_broadcast(&ft_cond);
                pthread_mutex_unlock(&ft_mutex);
                return 0;
            }

            if (rc != 0) {
                /* Already claimed (FT_RUNNING): job/ctx are in use on the
                   worker thread now, so it is no longer safe to abandon.
                   Fall back to an unbounded wait for FT_DONE. */
                use_timeout = 0;
            }
        } else {
            pthread_cond_wait(&ft_cond, &ft_mutex);
        }
    }

    completed = (ft_state == FT_DONE);
    ft_state = FT_IDLE;
    ft_job = NULL;
    pthread_cond_broadcast(&ft_cond);

    pthread_mutex_unlock(&ft_mutex);

    return completed;
}

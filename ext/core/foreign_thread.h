#ifndef SFML_RB_CORE_FOREIGN_THREAD_H
#define SFML_RB_CORE_FOREIGN_THREAD_H

/* SFML's audio thread is a foreign native thread Ruby never created, so it
   cannot legally call back into Ruby: rb_thread_call_with_gvl() requires the
   calling thread to have previously released the GVL via
   rb_thread_call_without_gvl(), which only a genuine Ruby thread can do -
   calling it from a foreign thread is a fatal VM error ("rb_thread_call_with_
   gvl() is called by non-ruby thread"). This module starts one persistent
   Ruby-owned worker thread that a foreign thread can safely hand jobs to. */

/* Starts the worker thread. Must be called once, with the GVL held, before
   any foreign thread calls run_on_ruby_thread(). */
void Init_ForeignThread(void);

/* Runs fn(ctx) on the worker thread and blocks the calling (foreign) thread
   until it completes, or until timeout_ms elapses (0 disables the timeout).
   Safe to call from any thread, including ones Ruby has never seen.

   On timeout, the job is only abandoned if the worker has not yet started
   running it; once running, this function waits for it to finish regardless
   of the timeout, so ctx is never touched by the worker after this function
   returns - the timeout exists to bound waiting on a merely slow-to-schedule
   worker (e.g. GVL contention), not to allow ctx to be freed out from under
   an in-flight job.

   Returns 1 if fn ran to completion, 0 if the job was abandoned on timeout
   before the worker claimed it. */
int run_on_ruby_thread(void (*fn)(void* ctx), void* ctx, unsigned long timeout_ms);

#endif // SFML_RB_CORE_FOREIGN_THREAD_H

#include "system/clock.h"

#include <stdio.h>

#include "system/time.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cClock;

static sfClock* Clock_create() {
    return sfClock_create();
}

static void Clock_free(void* ptr) {
    sfClock_destroy(ptr);
}

static const rb_data_type_t Clock_data_type = {
    .wrap_struct_name = "SF::System::Clock",
    .function = {.dmark = NULL, .dfree = Clock_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Clock_new_from(VALUE klass, sfClock* clock) {
    VALUE self;

    if (clock == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate clock");
    }

    self = TypedData_Wrap_Struct(klass, &Clock_data_type, clock);

    rb_obj_call_init(self, 0, NULL);

    return self;
}

static VALUE Clock_alloc(VALUE klass) {
    sfClock* clock = Clock_create();

    if (clock == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate clock");
    }

    return TypedData_Wrap_Struct(klass, &Clock_data_type, clock);
}

/* call-seq:
 *   Clock.new -> Clock
 *
 * Creates a new clock and starts it immediately.
 *
 * @return [Clock]
 */
static VALUE Clock_init(VALUE self) {
    return self;
}

/* call-seq: elapsed_time -> Time
 *
 * Returns the time elapsed on the clock.
 *
 * @return [Time] time elapsed since the clock was created, started, or last
 *   restarted/reset, whichever is most recent
 */
static VALUE Clock_get_elapsed_time(VALUE self) {
    return time_to_rb(sfClock_getElapsedTime(Get_Clock_Struct(self)));
}

/* call-seq: restart! -> Time
 *
 * Restarts the clock (equivalent to #reset! followed by #start!) and
 * returns the time elapsed before restarting.
 *
 * @return [Time]
 */
static VALUE Clock_restart(VALUE self) {
    return time_to_rb(sfClock_restart(Get_Clock_Struct(self)));
}

/* call-seq: reset! -> Time
 *
 * Stops the clock and resets its elapsed time to zero, returning the time
 * elapsed before resetting.
 *
 * @return [Time]
 */
static VALUE Clock_reset(VALUE self) {
    return time_to_rb(sfClock_reset(Get_Clock_Struct(self)));
}

/* call-seq: running? -> true or false
 *
 * Returns +true+ while the clock is running, +false+ once it is stopped.
 *
 * @return [Boolean]
 */
static VALUE Clock_is_running(VALUE self) {
    return BOOL2RB(sfClock_isRunning(Get_Clock_Struct(self)));
}

/* call-seq: start! -> self
 *
 * Resumes a stopped clock without resetting its elapsed time.
 *
 * @return [self]
 */
static VALUE Clock_start(VALUE self) {
    sfClock_start(Get_Clock_Struct(self));
    return self;
}

/* call-seq: stop! -> self
 *
 * Pauses the clock; #elapsed_time keeps returning the time at which it
 * was stopped until #start! or #restart! is called.
 *
 * @return [self]
 */
static VALUE Clock_stop(VALUE self) {
    sfClock_stop(Get_Clock_Struct(self));
    return self;
}

/* call-seq: copy -> Clock
 *
 * Returns an independent copy of the clock.
 *
 * @return [Clock] an independent copy with the same elapsed/running state
 */
static VALUE Clock_copy(VALUE self) {
    return Clock_new_from(Get_Klass_Clock(), sfClock_copy(Get_Clock_Struct(self)));
}

/* Document-class: SF::System::Clock
 * A stopwatch for measuring elapsed time.
 *
 * @!method self.measure { ... }
 *   Runs the block with a throwaway clock and returns the time it took.
 *   Rubyesque (Matz-like) over creating a Clock, yielding and reading
 *   #elapsed_time.
 *   @yield the work to time
 *   @return [Time] the elapsed time
 */
void Init_Clock(VALUE rb_mSystem) {
    rb_cClock = rb_define_class_under(rb_mSystem, "Clock", rb_cObject);

    rb_define_alloc_func(rb_cClock, Clock_alloc);

    // methods
    rb_define_method(rb_cClock, "initialize", Clock_init, 0);
    rb_define_method(rb_cClock, "restart!", Clock_restart, 0);
    rb_define_method(rb_cClock, "reset!", Clock_reset, 0);
    rb_define_method(rb_cClock, "start!", Clock_start, 0);
    rb_define_method(rb_cClock, "stop!", Clock_stop, 0);
    rb_define_method(rb_cClock, "copy", Clock_copy, 0);

    // getters
    rb_define_method(rb_cClock, "elapsed_time", Clock_get_elapsed_time, 0);
    rb_define_method(rb_cClock, "running?", Clock_is_running, 0);
}

void* Get_Clock_Struct(VALUE self) {
    sfClock* clock;
    TypedData_Get_Struct(self, sfClock, &Clock_data_type, clock);
    return clock;
}

VALUE Get_Klass_Clock() {
    return rb_cClock;
}

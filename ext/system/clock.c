#include "system/clock.h"

#include <stdio.h>

#include "system/time.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cClock;

static sfClock *Clock_create() {
    return sfClock_create();
}

static void Clock_free(void *ptr) {
    sfClock_destroy(ptr);
}

static const rb_data_type_t Clock_data_type = {
    .wrap_struct_name = "SFML::Clock",
    .function = {.dmark = NULL, .dfree = Clock_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE Clock_new_from(VALUE klass, sfClock *clock) {
    VALUE self;

    self = TypedData_Wrap_Struct(klass, &Clock_data_type, clock);

    rb_obj_call_init(self, 0, NULL);

    return self;
}

static VALUE Clock_new(VALUE klass) {
    return Clock_new_from(klass, Clock_create());
}

static VALUE Clock_init(VALUE self) {
    return self;
}

static VALUE Clock_get_elapsed_time(VALUE self) {
    return time_to_rb(sfClock_getElapsedTime(Get_Clock_Struct(self)));
}

static VALUE Clock_restart(VALUE self) {
    return time_to_rb(sfClock_restart(Get_Clock_Struct(self)));
}

static VALUE Clock_reset(VALUE self) {
    return time_to_rb(sfClock_reset(Get_Clock_Struct(self)));
}

static VALUE Clock_is_running(VALUE self) {
    return BOOL2RB(sfClock_isRunning(Get_Clock_Struct(self)));
}

static VALUE Clock_start(VALUE self) {
    sfClock_start(Get_Clock_Struct(self));
    return self;
}

static VALUE Clock_stop(VALUE self) {
    sfClock_stop(Get_Clock_Struct(self));
    return self;
}

static VALUE Clock_copy(VALUE self) {
    return Clock_new_from(Get_Klass_Clock(), sfClock_copy(Get_Clock_Struct(self)));
}

void Init_Clock(VALUE rb_module) {
    rb_cClock = rb_define_class_under(rb_module, "Clock", rb_cObject);

    rb_define_singleton_method(rb_cClock, "new", Clock_new, 0);

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

void *Get_Clock_Struct(VALUE self) {
    sfClock *clock;
    TypedData_Get_Struct(self, sfClock, &Clock_data_type, clock);
    return clock;
}

VALUE Get_Klass_Clock() {
    return rb_cClock;
}

#include "system/sleep.h"

#include <ruby.h>

#include "system/time.h"
#include "core/sfml.h"

static VALUE Sleep_sleep(VALUE module, VALUE rb_duration) {
    sfSleep(time_from_rb(rb_duration));

    return rb_duration;
}

void Init_Sleep(VALUE rb_module) {
    rb_define_module_function(rb_module, "sleep", Sleep_sleep, 1);
}

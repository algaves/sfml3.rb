#include "window/sensor.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "system/vec3.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE Sensor_is_available(VALUE module, VALUE rb_type) {
    return BOOL2RB(sfSensor_isAvailable(sensor_type_from_rb(rb_type)));
}

static VALUE Sensor_set_enabled(VALUE module, VALUE rb_type, VALUE rb_enabled) {
    sfSensor_setEnabled(sensor_type_from_rb(rb_type), RTEST(rb_enabled));
    return rb_enabled;
}

static VALUE Sensor_get_value(VALUE module, VALUE rb_type) {
    return vec3f_to_rb(sfSensor_getValue(sensor_type_from_rb(rb_type)));
}

void Init_Sensor(VALUE rb_module) {
    VALUE rb_mSensor = rb_define_module_under(rb_module, "Sensor");

    rb_define_module_function(rb_mSensor, "available?", Sensor_is_available, 1);
    rb_define_module_function(rb_mSensor, "set_enabled", Sensor_set_enabled, 2);
    rb_define_module_function(rb_mSensor, "enabled=", Sensor_set_enabled, 2);
    rb_define_module_function(rb_mSensor, "value", Sensor_get_value, 1);
}

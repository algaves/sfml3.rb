#include "window/sensor.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "system/vec3.h"
#include "core/macros.h"
#include "core/sfml.h"

/* call-seq:
 *   available?(type) -> true or false
 *
 * Returns +true+ if sensor +type+ is available on this device.
 *
 * @return [Boolean] whether sensor +type+ (a Symbol like
 *   +:accelerometer+, +:gyroscope+) is available on this device
 */
static VALUE Sensor_is_available(VALUE module, VALUE rb_type) {
    return BOOL2RB(sfSensor_isAvailable(sensor_type_from_rb(rb_type)));
}

/* call-seq:
 *   set_enabled(type, value) -> value
 *
 * Enables or disables sensor +type+. All sensors are disabled by default.
 *
 * @return [Boolean] +value+
 */
static VALUE Sensor_set_enabled(VALUE module, VALUE rb_type, VALUE rb_enabled) {
    sfSensor_setEnabled(sensor_type_from_rb(rb_type), RTEST(rb_enabled));
    return rb_enabled;
}

/* call-seq:
 *   value(type) -> Vector3
 *
 * Returns the current value of sensor +type+.
 *
 * @return [Vector3] the current value of sensor +type+
 */
static VALUE Sensor_get_value(VALUE module, VALUE rb_type) {
    return vec3f_to_rb(sfSensor_getValue(sensor_type_from_rb(rb_type)));
}

/* Document-module: SF::Window::Sensor
 * Access to hardware sensors (accelerometer, gyroscope, etc.), mainly
 * relevant on mobile platforms.
 *
 * @!method enable!(type)
 *   Enables sensor +type+. Rubyesque (Matz-like) over +set_enabled(type, true)+.
 *   @return [Boolean] +true+
 * @!method disable!(type)
 *   Disables sensor +type+. Rubyesque (Matz-like) over +set_enabled(type, false)+.
 *   @return [Boolean] +false+
 */
void Init_Sensor(VALUE rb_mWindow) {
    VALUE rb_mSensor = rb_define_module_under(rb_mWindow, "Sensor");

    rb_define_module_function(rb_mSensor, "available?", Sensor_is_available, 1);
    rb_define_module_function(rb_mSensor, "set_enabled", Sensor_set_enabled, 2);
    rb_define_module_function(rb_mSensor, "enabled=", Sensor_set_enabled, 2);
    rb_define_module_function(rb_mSensor, "value", Sensor_get_value, 1);
}

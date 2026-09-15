#include "window/joystick.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE Joystick_is_connected(VALUE module, VALUE rb_joystick) {
    return BOOL2RB(sfJoystick_isConnected(NUM2UINT(rb_joystick)));
}

static VALUE Joystick_get_button_count(VALUE module, VALUE rb_joystick) {
    return UINT2NUM(sfJoystick_getButtonCount(NUM2UINT(rb_joystick)));
}

static VALUE Joystick_has_axis(VALUE module, VALUE rb_joystick, VALUE rb_axis) {
    return BOOL2RB(sfJoystick_hasAxis(NUM2UINT(rb_joystick), joystick_axis_from_rb(rb_axis)));
}

static VALUE Joystick_is_button_pressed(VALUE module, VALUE rb_joystick, VALUE rb_button) {
    return BOOL2RB(sfJoystick_isButtonPressed(NUM2UINT(rb_joystick), NUM2UINT(rb_button)));
}

static VALUE Joystick_get_axis_position(VALUE module, VALUE rb_joystick, VALUE rb_axis) {
    return DBL2NUM(sfJoystick_getAxisPosition(NUM2UINT(rb_joystick), joystick_axis_from_rb(rb_axis)));
}

static VALUE Joystick_get_identification(VALUE module, VALUE rb_joystick) {
    sfJoystickIdentification identification = sfJoystick_getIdentification(NUM2UINT(rb_joystick));
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("name")),
                 rb_str_new_cstr(identification.name != NULL ? identification.name : ""));
    rb_hash_aset(hash, ID2SYM(rb_intern("vendor_id")), UINT2NUM(identification.vendorId));
    rb_hash_aset(hash, ID2SYM(rb_intern("product_id")), UINT2NUM(identification.productId));

    return hash;
}

static VALUE Joystick_update(VALUE module) {
    sfJoystick_update();
    return Qnil;
}

void Init_Joystick(VALUE rb_module) {
    VALUE rb_mJoystick = rb_define_module_under(rb_module, "Joystick");

    rb_define_const(rb_mJoystick, "COUNT", INT2NUM(sfJoystickCount));
    rb_define_const(rb_mJoystick, "BUTTON_COUNT", INT2NUM(sfJoystickButtonCount));
    rb_define_const(rb_mJoystick, "AXIS_COUNT", INT2NUM(sfJoystickAxisCount));

    rb_define_module_function(rb_mJoystick, "connected?", Joystick_is_connected, 1);
    rb_define_module_function(rb_mJoystick, "button_count", Joystick_get_button_count, 1);
    rb_define_module_function(rb_mJoystick, "has_axis?", Joystick_has_axis, 2);
    rb_define_module_function(rb_mJoystick, "button_pressed?", Joystick_is_button_pressed, 2);
    rb_define_module_function(rb_mJoystick, "axis_position", Joystick_get_axis_position, 2);
    rb_define_module_function(rb_mJoystick, "identification", Joystick_get_identification, 1);
    rb_define_module_function(rb_mJoystick, "update!", Joystick_update, 0);
}

#ifndef SFML_RB_WINDOW_INPUT_ENUMS_H
#define SFML_RB_WINDOW_INPUT_ENUMS_H

#include <ruby.h>

#include "core/sfml.h"

const char *mouse_button_name(sfMouseButton button);

sfMouseButton mouse_button_from_rb(VALUE rb_button);

const char *mouse_wheel_name(sfMouseWheel wheel);

sfMouseWheel mouse_wheel_from_rb(VALUE rb_wheel);

const char *joystick_axis_name(sfJoystickAxis axis);

sfJoystickAxis joystick_axis_from_rb(VALUE rb_axis);

const char *sensor_type_name(sfSensorType type);

sfSensorType sensor_type_from_rb(VALUE rb_type);

const char *cursor_type_name(sfCursorType type);

sfCursorType cursor_type_from_rb(VALUE rb_type);

uint32_t window_style_from_rb(VALUE rb_style);

VALUE window_style_to_rb(uint32_t style);

sfWindowState window_state_from_rb(VALUE rb_state);

VALUE window_state_to_rb(sfWindowState state);

#endif //SFML_RB_WINDOW_INPUT_ENUMS_H

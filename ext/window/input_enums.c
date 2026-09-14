#include "window/input_enums.h"

#include <ruby.h>
#include <string.h>

#include "core/exceptions.h"

static const char *mouse_button_names[] = {"left", "right", "middle", "extra1", "extra2"};
static const char *mouse_wheel_names[] = {"vertical", "horizontal"};
static const char *joystick_axis_names[] = {"x", "y", "z", "r", "u", "v", "pov_x", "pov_y"};
static const char *sensor_type_names[] = {
    "accelerometer", "gyroscope", "magnetometer", "gravity", "user_acceleration", "orientation"
};

static const char *cursor_type_names[] = {
    "arrow", "arrow_wait", "wait", "text", "hand", "size_horizontal", "size_vertical",
    "size_top_left_bottom_right", "size_bottom_left_top_right", "size_left", "size_right",
    "size_top", "size_bottom", "size_top_left", "size_bottom_right", "size_bottom_left",
    "size_top_right", "size_all", "cross", "help", "not_allowed"
};

/* Returns the matching index, or -1. Integer arguments pass through unchanged
   by the callers, so this only deals with symbols. */
static int symbol_index(VALUE rb_symbol, const char *const *names, size_t count) {
    const char *name;
    size_t i;

    if (!SYMBOL_P(rb_symbol)) {
        return -1;
    }

    name = rb_id2name(SYM2ID(rb_symbol));

    for (i = 0; i < count; i++) {
        if (strcmp(name, names[i]) == 0) {
            return (int) i;
        }
    }

    return -1;
}

const char *mouse_button_name(sfMouseButton button) {
    if (button >= 0 && button <= sfMouseButtonExtra2) {
        return mouse_button_names[button];
    }

    return "left";
}

sfMouseButton mouse_button_from_rb(VALUE rb_button) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_button)) {
        return (sfMouseButton) NUM2INT(rb_button);
    }

    index = symbol_index(rb_button, mouse_button_names, 5);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown mouse button");
    }

    return (sfMouseButton) index;
}

const char *mouse_wheel_name(sfMouseWheel wheel) {
    return (wheel == sfMouseHorizontalWheel) ? "horizontal" : "vertical";
}

sfMouseWheel mouse_wheel_from_rb(VALUE rb_wheel) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_wheel)) {
        return (sfMouseWheel) NUM2INT(rb_wheel);
    }

    index = symbol_index(rb_wheel, mouse_wheel_names, 2);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown mouse wheel");
    }

    return (sfMouseWheel) index;
}

const char *joystick_axis_name(sfJoystickAxis axis) {
    if (axis >= 0 && axis <= sfJoystickPovY) {
        return joystick_axis_names[axis];
    }

    return "x";
}

sfJoystickAxis joystick_axis_from_rb(VALUE rb_axis) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_axis)) {
        return (sfJoystickAxis) NUM2INT(rb_axis);
    }

    index = symbol_index(rb_axis, joystick_axis_names, 8);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown joystick axis");
    }

    return (sfJoystickAxis) index;
}

const char *sensor_type_name(sfSensorType type) {
    if (type >= 0 && type <= sfSensorOrientation) {
        return sensor_type_names[type];
    }

    return "accelerometer";
}

sfSensorType sensor_type_from_rb(VALUE rb_type) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_type)) {
        return (sfSensorType) NUM2INT(rb_type);
    }

    index = symbol_index(rb_type, sensor_type_names, 6);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown sensor type");
    }

    return (sfSensorType) index;
}

const char *cursor_type_name(sfCursorType type) {
    if (type >= 0 && type <= sfCursorNotAllowed) {
        return cursor_type_names[type];
    }

    return "arrow";
}

sfCursorType cursor_type_from_rb(VALUE rb_type) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_type)) {
        return (sfCursorType) NUM2INT(rb_type);
    }

    index = symbol_index(rb_type, cursor_type_names, 21);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown cursor type");
    }

    return (sfCursorType) index;
}

static uint32_t style_flag_from_symbol(VALUE rb_symbol) {
    const char *name;

    if (!SYMBOL_P(rb_symbol)) {
        rb_raise(rb_eArgError, "expected a window style symbol");
    }

    name = rb_id2name(SYM2ID(rb_symbol));

    if (strcmp(name, "none") == 0) {
        return sfNone;
    }

    if (strcmp(name, "titlebar") == 0) {
        return sfTitlebar;
    }

    if (strcmp(name, "resize") == 0) {
        return sfResize;
    }

    if (strcmp(name, "close") == 0) {
        return sfClose;
    }

    if (strcmp(name, "default") == 0) {
        return sfDefaultStyle;
    }

    rb_raise(rb_eArgError, "unknown window style: %s", name);
    return sfNone;
}

uint32_t window_style_from_rb(VALUE rb_style) {
    uint32_t style = 0;

    if (RB_INTEGER_TYPE_P(rb_style)) {
        return (uint32_t) NUM2UINT(rb_style);
    }

    if (SYMBOL_P(rb_style)) {
        return style_flag_from_symbol(rb_style);
    }

    if (RB_TYPE_P(rb_style, T_ARRAY)) {
        for (long i = 0; i < RARRAY_LEN(rb_style); i++) {
            style |= style_flag_from_symbol(rb_ary_entry(rb_style, i));
        }

        return style;
    }

    rb_raise(rb_eArgError, "expected a window style symbol or array of symbols");
    return sfNone;
}

VALUE window_style_to_rb(uint32_t style) {
    VALUE array = rb_ary_new();

    if (style == sfNone) {
        rb_ary_push(array, ID2SYM(rb_intern("none")));
        return array;
    }

    if ((style & sfTitlebar) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("titlebar")));
    }

    if ((style & sfResize) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("resize")));
    }

    if ((style & sfClose) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("close")));
    }

    return array;
}

sfWindowState window_state_from_rb(VALUE rb_state) {
    if (RB_INTEGER_TYPE_P(rb_state)) {
        return (sfWindowState) NUM2INT(rb_state);
    }

    if (SYMBOL_P(rb_state)) {
        const char *name = rb_id2name(SYM2ID(rb_state));

        if (strcmp(name, "fullscreen") == 0) {
            return sfFullscreen;
        }

        if (strcmp(name, "windowed") == 0) {
            return sfWindowed;
        }
    }

    rb_raise(rb_eArgError, "expected :windowed or :fullscreen");
    return sfWindowed;
}

VALUE window_state_to_rb(sfWindowState state) {
    return ID2SYM(rb_intern(state == sfFullscreen ? "fullscreen" : "windowed"));
}

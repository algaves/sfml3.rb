#include "window/mouse.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "window/window.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/sfml.h"

/* nil means desktop-relative. CSFML gives sfRenderWindow its own entry points,
   so a window argument never has to be cast down to sfWindowBase. */
static const sfRenderWindow* Mouse_relative_window(VALUE rb_window) {
    if (NIL_P(rb_window)) {
        return NULL;
    }

    if (!rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        raise_invalid_argument_class(Get_Klass_Window());
    }

    return Get_Window_Struct(rb_window);
}

static VALUE Mouse_is_button_pressed(VALUE module, VALUE rb_button) {
    return BOOL2RB(sfMouse_isButtonPressed(mouse_button_from_rb(rb_button)));
}

static VALUE Mouse_get_position(int argc, VALUE* argv, VALUE module) {
    VALUE rb_window;
    sfVector2i position;

    rb_scan_args(argc, argv, "01", &rb_window);
    if (NIL_P(rb_window)) {
        position = sfMouse_getPosition(NULL);
    } else {
        position = sfMouse_getPositionRenderWindow(Mouse_relative_window(rb_window));
    }

    return vec2f_to_rb((sfVector2f){(float)position.x, (float)position.y});
}

static VALUE Mouse_set_position(int argc, VALUE* argv, VALUE module) {
    VALUE rb_position, rb_window;

    rb_scan_args(argc, argv, "11", &rb_position, &rb_window);
    if (NIL_P(rb_window)) {
        sfMouse_setPosition(vec2i_from_rb(rb_position), NULL);
    } else {
        sfMouse_setPositionRenderWindow(vec2i_from_rb(rb_position),
                                        Mouse_relative_window(rb_window));
    }

    return rb_position;
}

static VALUE Mouse_position_eq(VALUE module, VALUE rb_position) {
    return Mouse_set_position(1, &rb_position, module);
}

void Init_Mouse(VALUE rb_module) {
    VALUE rb_mMouse = rb_define_module_under(rb_module, "Mouse");

    rb_define_module_function(rb_mMouse, "button_pressed?", Mouse_is_button_pressed, 1);
    rb_define_module_function(rb_mMouse, "pressed?", Mouse_is_button_pressed, 1);
    rb_define_module_function(rb_mMouse, "position", Mouse_get_position, -1);
    rb_define_module_function(rb_mMouse, "set_position", Mouse_set_position, -1);
    rb_define_module_function(rb_mMouse, "position=", Mouse_position_eq, 1);
}

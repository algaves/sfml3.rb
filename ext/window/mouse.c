#include "window/mouse.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "window/window.h"
#include "window/window_base.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/sfml.h"

/* A RenderWindow and a Window both wrap an sfRenderWindow; a WindowBase wraps
   an sfWindowBase. CSFML ships a separate entry point for each, so pick the one
   matching the handle -- casting an sfRenderWindow* down to sfWindowBase* would
   rely on the base being at offset 0, which CSFML does not guarantee. */
static sfVector2i Mouse_get_relative_position(VALUE rb_window) {
    if (rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        return sfMouse_getPositionRenderWindow(Get_Window_Struct(rb_window));
    }

    if (rb_obj_is_kind_of(rb_window, Get_Klass_WindowBase())) {
        return sfMouse_getPositionWindowBase(Get_WindowBase_Struct(rb_window));
    }

    raise_invalid_argument_class(Get_Klass_WindowBase());
    return (sfVector2i){0, 0};
}

static void Mouse_set_relative_position(VALUE rb_window, sfVector2i position) {
    if (rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        sfMouse_setPositionRenderWindow(position, Get_Window_Struct(rb_window));
        return;
    }

    if (rb_obj_is_kind_of(rb_window, Get_Klass_WindowBase())) {
        sfMouse_setPositionWindowBase(position, Get_WindowBase_Struct(rb_window));
        return;
    }

    raise_invalid_argument_class(Get_Klass_WindowBase());
}

/* call-seq:
 *   button_pressed?(button) -> true or false
 *
 * Returns +true+ if +button+ is currently pressed.
 *
 * @return [Boolean] whether +button+ (a Symbol like +:left+, +:right+,
 *   +:middle+, or an Integer) is currently pressed
 */
static VALUE Mouse_is_button_pressed(VALUE module, VALUE rb_button) {
    return BOOL2RB(sfMouse_isButtonPressed(mouse_button_from_rb(rb_button)));
}

/* call-seq:
 *   position(window = nil) -> Vector2
 *
 * Returns the mouse position in desktop coordinates, or relative to +window+.
 *
 * @return [Vector2] the mouse position in desktop coordinates, or relative
 *   to +window+'s client area when given
 */
static VALUE Mouse_get_position(int argc, VALUE* argv, VALUE module) {
    VALUE rb_window;
    sfVector2i position;

    rb_scan_args(argc, argv, "01", &rb_window);
    if (NIL_P(rb_window)) {
        position = sfMouse_getPosition(NULL);
    } else {
        position = Mouse_get_relative_position(rb_window);
    }

    return vec2f_to_rb((sfVector2f){(float)position.x, (float)position.y});
}

/* call-seq:
 *   set_position(position, window = nil) -> position
 *
 * Moves the mouse cursor to +position+, in desktop coordinates, or relative
 * to +window+'s client area when given.
 *
 * @return [Vector2] +position+
 */
static VALUE Mouse_set_position(int argc, VALUE* argv, VALUE module) {
    VALUE rb_position, rb_window;

    rb_scan_args(argc, argv, "11", &rb_position, &rb_window);
    if (NIL_P(rb_window)) {
        sfMouse_setPosition(vec2i_from_rb(rb_position), NULL);
    } else {
        Mouse_set_relative_position(rb_window, vec2i_from_rb(rb_position));
    }

    return rb_position;
}

/* call-seq:
 *   position=(value) -> value
 *
 * Moves the mouse cursor to +value+, in desktop coordinates.
 *
 * @return [Vector2] +value+
 */
static VALUE Mouse_position_eq(VALUE module, VALUE rb_position) {
    return Mouse_set_position(1, &rb_position, module);
}

/* Document-module: SFML::Mouse
 * Real-time mouse state and cursor positioning.
 */
void Init_Mouse(VALUE rb_mSFML) {
    VALUE rb_mMouse = rb_define_module_under(rb_mSFML, "Mouse");

    rb_define_module_function(rb_mMouse, "button_pressed?", Mouse_is_button_pressed, 1);
    rb_define_module_function(rb_mMouse, "pressed?", Mouse_is_button_pressed, 1);
    rb_define_module_function(rb_mMouse, "position", Mouse_get_position, -1);
    rb_define_module_function(rb_mMouse, "set_position", Mouse_set_position, -1);
    rb_define_module_function(rb_mMouse, "position=", Mouse_position_eq, 1);
}

#include "window/touch.h"

#include <ruby.h>

#include "window/window.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/sfml.h"

/* nil means desktop-relative. CSFML gives sfRenderWindow its own entry point,
   so a window argument never has to be cast down to sfWindowBase. */
static const sfRenderWindow* Touch_relative_window(VALUE rb_window) {
    if (NIL_P(rb_window)) {
        return NULL;
    }

    if (!rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        raise_invalid_argument_class(Get_Klass_Window());
    }

    return Get_Window_Struct(rb_window);
}

/* call-seq:
 *   down?(finger) -> true or false
 *
 * Returns +true+ if touch point +finger+ is currently down.
 *
 * @return [Boolean] whether touch point +finger+ is currently down
 */
static VALUE Touch_is_down(VALUE module, VALUE rb_finger) {
    return BOOL2RB(sfTouch_isDown(NUM2UINT(rb_finger)));
}

/* call-seq:
 *   position(finger, window = nil) -> Vector2
 *
 * Returns the position of touch point +finger+.
 *
 * @return [Vector2] the position of touch point +finger+, in desktop
 *   coordinates, or relative to +window+'s client area when given
 */
static VALUE Touch_get_position(int argc, VALUE* argv, VALUE module) {
    VALUE rb_finger, rb_window;
    sfVector2i position;

    rb_scan_args(argc, argv, "11", &rb_finger, &rb_window);
    if (NIL_P(rb_window)) {
        position = sfTouch_getPosition(NUM2UINT(rb_finger), NULL);
    } else {
        position =
            sfTouch_getPositionRenderWindow(NUM2UINT(rb_finger), Touch_relative_window(rb_window));
    }

    return vec2f_to_rb((sfVector2f){(float)position.x, (float)position.y});
}

/* Document-module: SFML::Touch
 * Real-time touch-screen state.
 */
void Init_Touch(VALUE rb_mSFML) {
    VALUE rb_mTouch = rb_define_module_under(rb_mSFML, "Touch");

    rb_define_module_function(rb_mTouch, "down?", Touch_is_down, 1);
    rb_define_module_function(rb_mTouch, "position", Touch_get_position, -1);
}

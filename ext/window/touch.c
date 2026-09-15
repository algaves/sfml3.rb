#include "window/touch.h"

#include <ruby.h>

#include "window/window.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/sfml.h"

static const sfWindowBase *Touch_relative_window(VALUE rb_window) {
    if (NIL_P(rb_window)) {
        return NULL;
    }

    if (!rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        raise_invalid_argument_class(Get_Klass_Window());
    }

    return (const sfWindowBase *) Get_Window_Struct(rb_window);
}

static VALUE Touch_is_down(VALUE module, VALUE rb_finger) {
    return BOOL2RB(sfTouch_isDown(NUM2UINT(rb_finger)));
}

static VALUE Touch_get_position(int argc, VALUE *argv, VALUE module) {
    VALUE rb_finger, rb_window;
    sfVector2i position;

    rb_scan_args(argc, argv, "11", &rb_finger, &rb_window);
    position = sfTouch_getPositionWindowBase(NUM2UINT(rb_finger), Touch_relative_window(rb_window));

    return vec2f_to_rb((sfVector2f) {(float) position.x, (float) position.y});
}

void Init_Touch(VALUE rb_module) {
    VALUE rb_mTouch = rb_define_module_under(rb_module, "Touch");

    rb_define_module_function(rb_mTouch, "down?", Touch_is_down, 1);
    rb_define_module_function(rb_mTouch, "position", Touch_get_position, -1);
}

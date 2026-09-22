#include "graphics/render_window.h"

#include <ruby.h>

#include "graphics/render_state.h"
#include "graphics/target.h"
#include "window/window.h"
#include "window/window_base.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cRenderWindow;

/* call-seq:
 *   draw(drawable, state = nil) -> self
 *
 * Draws +drawable+ into the window. Unlike Window#draw this hands +self+ to the
 * drawable, because a RenderWindow is a RenderTarget.
 *
 * @return [self]
 * @raise [ArgumentError] if given no arguments or more than 2
 */
static VALUE RenderWindow_draw(int argc, VALUE* argv, VALUE self) {
    VALUE rb_drawable, rb_state;

    if (argc == 0 || argc > 2) {
        raise_invalid_arguments_excepted(-1, argc);
    }

    rb_drawable = argv[0];
    rb_state = (argc == 2) ? argv[1] : Get_New_RenderState();

    rb_funcall(rb_drawable, rb_intern("draw"), 2, self, rb_state);

    return self;
}

/* Document-class: SF::Graphics::RenderWindow
 * A SF::Window::Window that is also a SF::Graphics::RenderTarget, so it can be handed
 * directly to a drawable's #draw as well as drawn into with #draw. Creation,
 * events and every window method come from Window (and WindowBase); the
 * render-target surface is listed on SF::Graphics::RenderTarget.
 */
void Init_RenderWindow(VALUE rb_mGraphics) {
    rb_cRenderWindow = rb_define_class_under(rb_mGraphics, "RenderWindow", Get_Klass_Window());

    rb_define_alloc_func(rb_cRenderWindow, Window_alloc);
    rb_define_method(rb_cRenderWindow, "draw", RenderWindow_draw, -1);

    rb_include_module(rb_cRenderWindow, Get_Module_RenderTarget());
}

VALUE Get_Klass_RenderWindow(void) {
    return rb_cRenderWindow;
}

#include "graphics/drawable.h"

#include <ruby.h>
#include <stdio.h>

#include "graphics/transform.h"
#include "graphics/transformable.h"
#include "graphics/render_state.h"
#include "graphics/target.h"
#include "window/window.h"
#include "core/exceptions.h"
#include "system/vec2.h"
#include "graphics/rect.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_mDrawable;

/* call-seq:
 *   draw(target, state) -> nil
 *
 * Default no-op implementation; classes that include Drawable (Circle,
 * RectangleShape, ConvexShape, Shape, Sprite, Text, VertexArray,
 * VertexBuffer, ...) override this with the actual drawing call.
 *
 * @return [nil]
 */
static VALUE Drawable_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
    }

    return Qnil;
}

/* Document-module: SFML::Drawable
 * A mixin for anything that can be drawn to a render target (Window,
 * RenderTexture) via #draw(target, state).
 */
void Init_Drawable(VALUE rb_mSFML) {
    rb_mDrawable = rb_define_module_under(rb_mSFML, "Drawable");

    rb_define_method(rb_mDrawable, "draw", Drawable_draw, 2);
}

VALUE Get_Module_Drawable() {
    return rb_mDrawable;
}
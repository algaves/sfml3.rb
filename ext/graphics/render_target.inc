/* Shared method bodies for every render target (Window, RenderTexture).
   Included from two translation units; each defines RT_FN, RT_METHOD and
   RT_HANDLE before including this file, then undefines them after. Generating
   concrete methods per class keeps the compiler's type checks on the handle and
   on every sf* call, which a runtime dispatch through void* would not.

   CSFML gives sfRenderWindow and sfRenderTexture identical render-target
   surfaces under two name prefixes, so RT_FN is the only thing that differs.
   SFML::Target still exists for the runtime-dispatch case (Drawable#draw); this
   file is about the methods each concrete target owns directly. */

#include "graphics/enums.h"
#include "graphics/render_state.h"
#include "graphics/vertex.h"
#include "graphics/vertex_buffer.h"
#include "graphics/view.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"

/* A nil view means "the target's current view", which is what CSFML's NULL
   means for every one of these. */
static const sfView *RT_METHOD(view_arg)(VALUE rb_view) {
    if (NIL_P(rb_view)) {
        return NULL;
    }

    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        raise_invalid_argument_class(Get_Klass_View());
    }

    return Get_View_Struct(rb_view);
}

static const sfRenderStates *RT_METHOD(state_arg)(VALUE rb_state) {
    if (NIL_P(rb_state)) {
        return NULL;
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    return Get_RenderState_Struct(rb_state);
}

static VALUE RT_METHOD(is_srgb)(VALUE self) {
    return BOOL2RB(RT_FN(isSrgb)(RT_HANDLE(self)));
}

static VALUE RT_METHOD(clear_stencil)(VALUE self, VALUE rb_value) {
    sfStencilValue value = {.value = (unsigned int) NUM2UINT(rb_value)};

    RT_FN(clearStencil)(RT_HANDLE(self), value);

    return self;
}

static VALUE RT_METHOD(clear_color_and_stencil)(VALUE self, VALUE rb_color, VALUE rb_stencil) {
    sfStencilValue value = {.value = (unsigned int) NUM2UINT(rb_stencil)};

    RT_FN(clearColorAndStencil)(RT_HANDLE(self), color_from_rb(rb_color), value);

    return self;
}

static VALUE RT_METHOD(get_viewport)(int argc, VALUE *argv, VALUE self) {
    VALUE rb_view;

    rb_scan_args(argc, argv, "01", &rb_view);

    return int_rect_to_rb(RT_FN(getViewport)(RT_HANDLE(self), RT_METHOD(view_arg)(rb_view)));
}

static VALUE RT_METHOD(get_scissor)(int argc, VALUE *argv, VALUE self) {
    VALUE rb_view;

    rb_scan_args(argc, argv, "01", &rb_view);

    return int_rect_to_rb(RT_FN(getScissor)(RT_HANDLE(self), RT_METHOD(view_arg)(rb_view)));
}

/* Pixel -> world. The inverse of the view transform, so a zoomed, rotated or
   viewport-shifted view still turns a mouse position into scene coordinates. */
static VALUE RT_METHOD(map_pixel_to_coords)(int argc, VALUE *argv, VALUE self) {
    VALUE rb_point, rb_view;

    rb_scan_args(argc, argv, "11", &rb_point, &rb_view);

    return vec2f_to_rb(RT_FN(mapPixelToCoords)(RT_HANDLE(self),
                                               vec2i_from_rb(rb_point),
                                               RT_METHOD(view_arg)(rb_view)));
}

/* World -> pixel. Returns integer pixel coordinates. */
static VALUE RT_METHOD(map_coords_to_pixel)(int argc, VALUE *argv, VALUE self) {
    VALUE rb_point, rb_view;
    sfVector2i pixel;

    rb_scan_args(argc, argv, "11", &rb_point, &rb_view);

    pixel = RT_FN(mapCoordsToPixel)(RT_HANDLE(self),
                                    vec2f_from_rb(rb_point),
                                    RT_METHOD(view_arg)(rb_view));

    return VEC2_C2RB(pixel);
}

static VALUE RT_METHOD(push_gl_states)(VALUE self) {
    RT_FN(pushGLStates)(RT_HANDLE(self));
    return self;
}

static VALUE RT_METHOD(pop_gl_states)(VALUE self) {
    RT_FN(popGLStates)(RT_HANDLE(self));
    return self;
}

static VALUE RT_METHOD(reset_gl_states)(VALUE self) {
    RT_FN(resetGLStates)(RT_HANDLE(self));
    return self;
}

/* draw_primitives(vertices, primitive, state = nil) */
static VALUE RT_METHOD(draw_primitives)(int argc, VALUE *argv, VALUE self) {
    VALUE rb_vertices, rb_primitive, rb_state;
    sfPrimitiveType type;
    const sfRenderStates *states;
    sfVertex *vertices;
    size_t count;

    rb_scan_args(argc, argv, "21", &rb_vertices, &rb_primitive, &rb_state);

    /* Every argument is converted before the array is allocated, so a bad
       vertex raises without leaking it. */
    type = primitive_type_from_rb(rb_primitive);
    states = RT_METHOD(state_arg)(rb_state);
    vertices = vertices_from_rb(rb_vertices, &count);

    RT_FN(drawPrimitives)(RT_HANDLE(self), vertices, count, type, states);

    xfree(vertices);

    return self;
}

/* draw_vertex_buffer_range(buffer, first, count, state = nil) */
static VALUE RT_METHOD(draw_vertex_buffer_range)(int argc, VALUE *argv, VALUE self) {
    VALUE rb_buffer, rb_first, rb_count, rb_state;

    rb_scan_args(argc, argv, "31", &rb_buffer, &rb_first, &rb_count, &rb_state);

    if (!rb_obj_is_kind_of(rb_buffer, Get_Klass_VertexBuffer())) {
        raise_invalid_argument_class(Get_Klass_VertexBuffer());
    }

    RT_FN(drawVertexBufferRange)(RT_HANDLE(self),
                                 Get_VertexBuffer_Struct(rb_buffer),
                                 NUM2SIZET(rb_first),
                                 NUM2SIZET(rb_count),
                                 RT_METHOD(state_arg)(rb_state));

    return self;
}

static void RT_METHOD(define_render_target_methods)(VALUE klass) {
    rb_define_method(klass, "srgb?", RT_METHOD(is_srgb), 0);
    rb_define_method(klass, "clear_stencil", RT_METHOD(clear_stencil), 1);
    rb_define_method(klass, "clear_color_and_stencil", RT_METHOD(clear_color_and_stencil), 2);
    rb_define_method(klass, "viewport", RT_METHOD(get_viewport), -1);
    rb_define_method(klass, "scissor", RT_METHOD(get_scissor), -1);
    rb_define_method(klass, "map_pixel_to_coords", RT_METHOD(map_pixel_to_coords), -1);
    rb_define_method(klass, "map_coords_to_pixel", RT_METHOD(map_coords_to_pixel), -1);
    rb_define_method(klass, "push_gl_states", RT_METHOD(push_gl_states), 0);
    rb_define_method(klass, "pop_gl_states", RT_METHOD(pop_gl_states), 0);
    rb_define_method(klass, "reset_gl_states", RT_METHOD(reset_gl_states), 0);
    rb_define_method(klass, "draw_primitives", RT_METHOD(draw_primitives), -1);
    rb_define_method(klass, "draw_vertex_buffer_range", RT_METHOD(draw_vertex_buffer_range), -1);
}

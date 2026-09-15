#include "graphics/render_texture.h"

#include <ruby.h>

#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/view.h"
#include "graphics/texture.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cRenderTexture;

static void RenderTexture_free(void *ptr) {
    sfRenderTexture_destroy(ptr);
}

static const rb_data_type_t RenderTexture_data_type = {
    .wrap_struct_name = "SFML::RenderTexture",
    .function = {.dmark = NULL, .dfree = RenderTexture_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

sfRenderTexture *Get_RenderTexture_Struct(VALUE self) {
    sfRenderTexture *ptr;
    TypedData_Get_Struct(self, sfRenderTexture, &RenderTexture_data_type, ptr);
    return ptr;
}

static VALUE RenderTexture_wrap(VALUE klass, sfRenderTexture *render_texture) {
    if (render_texture == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create render texture");
    }

    return TypedData_Wrap_Struct(klass, &RenderTexture_data_type, render_texture);
}

static VALUE RenderTexture_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_size, rb_settings;

    rb_scan_args(argc, argv, "11", &rb_size, &rb_settings);

    (void) rb_settings;

    return RenderTexture_wrap(klass, sfRenderTexture_create(vec2u_from_rb(rb_size), NULL));
}

static VALUE RenderTexture_get_size(VALUE self) {
    sfVector2u size = sfRenderTexture_getSize(Get_RenderTexture_Struct(self));

    return vec2f_to_rb((sfVector2f) {(float) size.x, (float) size.y});
}

static VALUE RenderTexture_is_srgb(VALUE self) {
    return BOOL2RB(sfRenderTexture_isSrgb(Get_RenderTexture_Struct(self)));
}

static VALUE RenderTexture_set_active(VALUE self, VALUE rb_active) {
    return BOOL2RB(sfRenderTexture_setActive(Get_RenderTexture_Struct(self), RTEST(rb_active)));
}

static VALUE RenderTexture_display(VALUE self) {
    sfRenderTexture_display(Get_RenderTexture_Struct(self));
    return self;
}

static VALUE RenderTexture_clear(int argc, VALUE *argv, VALUE self) {
    VALUE rb_color;
    sfColor color = sfBlack;

    rb_scan_args(argc, argv, "01", &rb_color);

    if (!NIL_P(rb_color)) {
        color = color_from_rb(rb_color);
    }

    sfRenderTexture_clear(Get_RenderTexture_Struct(self), color);

    return self;
}

static VALUE RenderTexture_clear_stencil(VALUE self, VALUE rb_value) {
    sfStencilValue value = {.value = (unsigned int) NUM2UINT(rb_value)};

    sfRenderTexture_clearStencil(Get_RenderTexture_Struct(self), value);

    return self;
}

static VALUE RenderTexture_clear_color_and_stencil(VALUE self, VALUE rb_color, VALUE rb_stencil) {
    sfStencilValue value = {.value = (unsigned int) NUM2UINT(rb_stencil)};

    sfRenderTexture_clearColorAndStencil(Get_RenderTexture_Struct(self), color_from_rb(rb_color), value);

    return self;
}

static VALUE RenderTexture_set_view(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        raise_invalid_argument_class(Get_Klass_View());
    }

    sfRenderTexture_setView(Get_RenderTexture_Struct(self), Get_View_Struct(rb_view));

    return self;
}

static VALUE RenderTexture_get_view(VALUE self) {
    return Get_Casting_View(sfView_copy(sfRenderTexture_getView(Get_RenderTexture_Struct(self))));
}

static VALUE RenderTexture_get_default_view(VALUE self) {
    return Get_Casting_View(sfView_copy(sfRenderTexture_getDefaultView(Get_RenderTexture_Struct(self))));
}

static VALUE RenderTexture_get_viewport(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        raise_invalid_argument_class(Get_Klass_View());
    }

    return int_rect_to_rb(sfRenderTexture_getViewport(Get_RenderTexture_Struct(self), Get_View_Struct(rb_view)));
}

static VALUE RenderTexture_get_scissor(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        raise_invalid_argument_class(Get_Klass_View());
    }

    return int_rect_to_rb(sfRenderTexture_getScissor(Get_RenderTexture_Struct(self), Get_View_Struct(rb_view)));
}

static VALUE RenderTexture_get_texture(VALUE self) {
    return texture_from_borrowed(sfRenderTexture_getTexture(Get_RenderTexture_Struct(self)));
}

static VALUE RenderTexture_set_smooth(VALUE self, VALUE rb_smooth) {
    sfRenderTexture_setSmooth(Get_RenderTexture_Struct(self), RTEST(rb_smooth));
    return rb_smooth;
}

static VALUE RenderTexture_is_smooth(VALUE self) {
    return BOOL2RB(sfRenderTexture_isSmooth(Get_RenderTexture_Struct(self)));
}

static VALUE RenderTexture_set_repeated(VALUE self, VALUE rb_repeated) {
    sfRenderTexture_setRepeated(Get_RenderTexture_Struct(self), RTEST(rb_repeated));
    return rb_repeated;
}

static VALUE RenderTexture_is_repeated(VALUE self) {
    return BOOL2RB(sfRenderTexture_isRepeated(Get_RenderTexture_Struct(self)));
}

static VALUE RenderTexture_generate_mipmap(VALUE self) {
    return BOOL2RB(sfRenderTexture_generateMipmap(Get_RenderTexture_Struct(self)));
}

static VALUE RenderTexture_draw(int argc, VALUE *argv, VALUE self) {
    VALUE rb_drawable, rb_state;

    if (argc == 0 || argc > 2) {
        raise_invalid_arguments_excepted(-1, argc);
    }

    rb_drawable = argv[0];
    rb_state = (argc == 2) ? argv[1] : Get_New_RenderState();

    rb_funcall(Get_New_Target_From_RenderTexture(self), rb_intern("draw"), 2, rb_drawable, rb_state);

    return self;
}

static VALUE RenderTexture_maximum_antialiasing_level(VALUE klass) {
    return UINT2NUM(sfRenderTexture_getMaximumAntiAliasingLevel());
}

void Init_RenderTexture(VALUE rb_module) {
    rb_cRenderTexture = rb_define_class_under(rb_module, "RenderTexture", rb_cObject);

    rb_define_singleton_method(rb_cRenderTexture, "new", RenderTexture_new, -1);
    rb_define_singleton_method(rb_cRenderTexture, "maximum_antialiasing_level",
                               RenderTexture_maximum_antialiasing_level, 0);

    rb_define_method(rb_cRenderTexture, "size", RenderTexture_get_size, 0);
    rb_define_method(rb_cRenderTexture, "srgb?", RenderTexture_is_srgb, 0);
    rb_define_method(rb_cRenderTexture, "display", RenderTexture_display, 0);
    rb_define_method(rb_cRenderTexture, "clear", RenderTexture_clear, -1);
    rb_define_method(rb_cRenderTexture, "clear_stencil", RenderTexture_clear_stencil, 1);
    rb_define_method(rb_cRenderTexture, "clear_color_and_stencil", RenderTexture_clear_color_and_stencil, 2);
    rb_define_method(rb_cRenderTexture, "view", RenderTexture_get_view, 0);
    rb_define_method(rb_cRenderTexture, "default_view", RenderTexture_get_default_view, 0);
    rb_define_method(rb_cRenderTexture, "viewport", RenderTexture_get_viewport, 1);
    rb_define_method(rb_cRenderTexture, "scissor", RenderTexture_get_scissor, 1);
    rb_define_method(rb_cRenderTexture, "texture", RenderTexture_get_texture, 0);
    rb_define_method(rb_cRenderTexture, "smooth?", RenderTexture_is_smooth, 0);
    rb_define_method(rb_cRenderTexture, "repeated?", RenderTexture_is_repeated, 0);
    rb_define_method(rb_cRenderTexture, "generate_mipmap", RenderTexture_generate_mipmap, 0);

    rb_define_method(rb_cRenderTexture, "view=", RenderTexture_set_view, 1);
    rb_define_method(rb_cRenderTexture, "active=", RenderTexture_set_active, 1);
    rb_define_method(rb_cRenderTexture, "smooth=", RenderTexture_set_smooth, 1);
    rb_define_method(rb_cRenderTexture, "repeated=", RenderTexture_set_repeated, 1);

    rb_define_method(rb_cRenderTexture, "draw", RenderTexture_draw, -1);
}

VALUE Get_Klass_RenderTexture(void) {
    return rb_cRenderTexture;
}

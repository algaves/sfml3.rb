#include "graphics/view.h"

#include <stdio.h>

#include "graphics/transform.h"
#include "graphics/transformable.h"
#include "graphics/color.h"
#include "system/vec2.h"
#include "graphics/rect.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cView;

static sfView* View_create() {
    return sfView_create();
}

static void View_free(void* ptr) {
    sfView_destroy(ptr);
}

static const rb_data_type_t View_data_type = {
    .wrap_struct_name = "SFML::View",
    .function = {.dmark = NULL, .dfree = View_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE View_new_from(VALUE klass, sfView* c_view) {
    VALUE self;
    sfView* view;

    if (c_view != NULL) {
        view = c_view;
    } else {
        view = View_create();
    }

    self = TypedData_Wrap_Struct(klass, &View_data_type, view);

    rb_obj_call_init(self, 0, NULL);

    return self;
}

static VALUE View_new(VALUE klass) {
    return View_new_from(klass, NULL);
}

/* The rect is the visible area: position is the top-left corner, not the
   centre, which is what distinguishes this from new + center=/size=. */
static VALUE View_s_from_rect(VALUE klass, VALUE rb_rect) {
    return View_new_from(klass, sfView_createFromRect(rect_from_rb(rb_rect)));
}

static VALUE View_init(VALUE self) {
    return self;
}

static VALUE View_set_rotation(VALUE self, VALUE rb_rotation) {
    sfView_setRotation(Get_View_Struct(self), NUM2DBL(rb_rotation));
    return self;
}

static VALUE View_set_size(VALUE self, VALUE rb_scale) {
    sfView_setSize(Get_View_Struct(self), vec2f_from_rb(rb_scale));
    return self;
}

static VALUE View_set_center(VALUE self, VALUE rb_origin) {
    sfVector2f origin = VEC2_RB2C(rb_origin);
    sfView_setCenter(Get_View_Struct(self), origin);
    return self;
}

static VALUE View_set_viewport(VALUE self, VALUE rb_viewport) {
    sfFloatRect viewport = RECT_RB2C(rb_viewport);
    sfView_setViewport(Get_View_Struct(self), viewport);

    return self;
}

static VALUE View_get_rotation(VALUE self) {
    return DBL2NUM(sfView_getRotation(Get_View_Struct(self)));
}

static VALUE View_get_size(VALUE self) {
    return vec2f_to_rb(sfView_getSize(Get_View_Struct(self)));
}

static VALUE View_get_center(VALUE self) {
    return vec2f_to_rb(sfView_getCenter(Get_View_Struct(self)));
}

static VALUE View_get_viewport(VALUE self) {
    return RECT_C2RB(sfView_getViewport(Get_View_Struct(self)));
}

static VALUE View_set_scissor(VALUE self, VALUE rb_scissor) {
    sfView_setScissor(Get_View_Struct(self), RECT_RB2C(rb_scissor));

    return self;
}

static VALUE View_get_scissor(VALUE self) {
    return RECT_C2RB(sfView_getScissor(Get_View_Struct(self)));
}

static VALUE View_move(VALUE self, VALUE rb_move) {
    sfView_move(Get_View_Struct(self), vec2f_from_rb(rb_move));
    return self;
}

static VALUE View_rotate(VALUE self, VALUE rb_angle) {
    sfView_rotate(Get_View_Struct(self), NUM2DBL(rb_angle));

    return self;
}

static VALUE View_zoom(VALUE self, VALUE rb_zoom) {
    sfView_zoom(Get_View_Struct(self), NUM2DBL(rb_zoom));

    return self;
}

static VALUE View_copy(VALUE self) {
    return Get_Casting_View(sfView_copy(Get_View_Struct(self)));
}

void Init_View(VALUE rb_module) {
    rb_cView = rb_define_class_under(rb_module, "View", rb_cObject);

    rb_define_singleton_method(rb_cView, "new", View_new, 0);
    rb_define_singleton_method(rb_cView, "from_rect", View_s_from_rect, 1);

    // methods
    rb_define_method(rb_cView, "initialize", View_init, 0);
    rb_define_method(rb_cView, "move", View_move, 1);
    rb_define_method(rb_cView, "rotate", View_rotate, 1);
    rb_define_method(rb_cView, "zoom", View_zoom, 1);
    rb_define_method(rb_cView, "copy", View_copy, 0);

    // setters
    rb_define_method(rb_cView, "rotation=", View_set_rotation, 1);
    rb_define_method(rb_cView, "size=", View_set_size, 1);
    rb_define_method(rb_cView, "center=", View_set_center, 1);
    rb_define_method(rb_cView, "viewport=", View_set_viewport, 1);
    rb_define_method(rb_cView, "scissor=", View_set_scissor, 1);

    // getters
    rb_define_method(rb_cView, "rotation", View_get_rotation, 0);
    rb_define_method(rb_cView, "size", View_get_size, 0);
    rb_define_method(rb_cView, "center", View_get_center, 0);
    rb_define_method(rb_cView, "viewport", View_get_viewport, 0);
    rb_define_method(rb_cView, "scissor", View_get_scissor, 0);
}

void* Get_View_Struct(VALUE self) {
    sfView* ptr;
    TypedData_Get_Struct(self, sfView, &View_data_type, ptr);
    return ptr;
}

VALUE Get_Klass_View() {
    return rb_cView;
}

VALUE Get_Casting_View(void* ptr) {
    return View_new_from(Get_Klass_View(), ptr);
}
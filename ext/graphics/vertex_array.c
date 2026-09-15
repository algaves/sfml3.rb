#include "graphics/vertex_array.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/drawable.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/rect.h"
#include "graphics/vertex.h"
#include "graphics/enums.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cVertexArray;

static void VertexArray_free(void *ptr) {
    sfVertexArray_destroy(ptr);
}

static const rb_data_type_t VertexArray_data_type = {
    .wrap_struct_name = "SFML::VertexArray",
    .function = {.dmark = NULL, .dfree = VertexArray_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE VertexArray_wrap(VALUE klass, sfVertexArray *array) {
    if (array == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create vertex array");
    }

    return TypedData_Wrap_Struct(klass, &VertexArray_data_type, array);
}

static sfVertexArray *Get_VertexArray_Struct(VALUE self) {
    sfVertexArray *ptr;
    TypedData_Get_Struct(self, sfVertexArray, &VertexArray_data_type, ptr);
    return ptr;
}

static VALUE VertexArray_new(VALUE klass) {
    return VertexArray_wrap(klass, sfVertexArray_create());
}

static VALUE VertexArray_copy(VALUE self) {
    return VertexArray_wrap(Get_Klass_VertexArray(), sfVertexArray_copy(Get_VertexArray_Struct(self)));
}

static VALUE VertexArray_get_vertex_count(VALUE self) {
    return SIZET2NUM(sfVertexArray_getVertexCount(Get_VertexArray_Struct(self)));
}

static VALUE VertexArray_get_vertex(VALUE self, VALUE rb_index) {
    return vertex_to_rb(*sfVertexArray_getVertex(Get_VertexArray_Struct(self), (size_t) NUM2SIZET(rb_index)));
}

static VALUE VertexArray_set_vertex(VALUE self, VALUE rb_index, VALUE rb_vertex) {
    *sfVertexArray_getVertex(Get_VertexArray_Struct(self), (size_t) NUM2SIZET(rb_index)) = vertex_from_rb(rb_vertex);
    return rb_vertex;
}

static VALUE VertexArray_append(VALUE self, VALUE rb_vertex) {
    sfVertexArray_append(Get_VertexArray_Struct(self), vertex_from_rb(rb_vertex));
    return self;
}

static VALUE VertexArray_clear(VALUE self) {
    sfVertexArray_clear(Get_VertexArray_Struct(self));
    return self;
}

static VALUE VertexArray_resize(VALUE self, VALUE rb_count) {
    sfVertexArray_resize(Get_VertexArray_Struct(self), (size_t) NUM2SIZET(rb_count));
    return self;
}

static VALUE VertexArray_get_primitive_type(VALUE self) {
    return ID2SYM(rb_intern(primitive_type_name(sfVertexArray_getPrimitiveType(Get_VertexArray_Struct(self)))));
}

static VALUE VertexArray_set_primitive_type(VALUE self, VALUE rb_type) {
    sfVertexArray_setPrimitiveType(Get_VertexArray_Struct(self), primitive_type_from_rb(rb_type));
    return rb_type;
}

static VALUE VertexArray_get_bounds(VALUE self) {
    return rect_to_rb(sfVertexArray_getBounds(Get_VertexArray_Struct(self)));
}

static VALUE VertexArray_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawVertexArray, sfRenderTexture_drawVertexArray,
                Get_VertexArray_Struct(self), Get_RenderState_Struct(rb_state));

    return Qnil;
}

void Init_VertexArray(VALUE rb_module) {
    rb_cVertexArray = rb_define_class_under(rb_module, "VertexArray", rb_cObject);

    rb_include_module(rb_cVertexArray, Get_Module_Drawable());

    rb_define_singleton_method(rb_cVertexArray, "new", VertexArray_new, 0);

    rb_define_method(rb_cVertexArray, "copy", VertexArray_copy, 0);
    rb_define_method(rb_cVertexArray, "vertex_count", VertexArray_get_vertex_count, 0);
    rb_define_method(rb_cVertexArray, "vertex", VertexArray_get_vertex, 1);
    rb_define_method(rb_cVertexArray, "set_vertex", VertexArray_set_vertex, 2);
    rb_define_method(rb_cVertexArray, "append", VertexArray_append, 1);
    rb_define_method(rb_cVertexArray, "clear!", VertexArray_clear, 0);
    rb_define_method(rb_cVertexArray, "resize", VertexArray_resize, 1);
    rb_define_method(rb_cVertexArray, "primitive", VertexArray_get_primitive_type, 0);
    rb_define_method(rb_cVertexArray, "primitive=", VertexArray_set_primitive_type, 1);
    rb_define_method(rb_cVertexArray, "bounds", VertexArray_get_bounds, 0);
    rb_define_method(rb_cVertexArray, "draw", VertexArray_draw, 2);
}

VALUE Get_Klass_VertexArray(void) {
    return rb_cVertexArray;
}

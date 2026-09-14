#include "graphics/rectangle.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/circle.h"
#include "graphics/drawable.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/transform.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "graphics/texture.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfRectangleShape *shape;
    VALUE rb_texture;
} RectangleShape;

static VALUE rb_cRectangleShape;

static void RectangleShape_mark(void *ptr) {
    rb_gc_mark(((RectangleShape *) ptr)->rb_texture);
}

static void RectangleShape_free(void *ptr) {
    sfRectangleShape_destroy(((RectangleShape *) ptr)->shape);
    free(ptr);
}

static const rb_data_type_t RectangleShape_data_type = {
    .wrap_struct_name = "SFML::RectangleShape",
    .function = {.dmark = RectangleShape_mark, .dfree = RectangleShape_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static RectangleShape *Get_RectangleShape(VALUE self) {
    RectangleShape *ptr;
    TypedData_Get_Struct(self, RectangleShape, &RectangleShape_data_type, ptr);
    return ptr;
}

static sfRectangleShape *Get_RectangleShape_Struct(VALUE self) {
    return Get_RectangleShape(self)->shape;
}

static VALUE RectangleShape_wrap(VALUE klass, sfRectangleShape *shape) {
    RectangleShape *ptr;

    if (shape == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create rectangle shape");
    }

    ptr = malloc(sizeof(RectangleShape));
    ptr->shape = shape;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &RectangleShape_data_type, ptr);
}

static VALUE RectangleShape_new(int argc, VALUE *argv, VALUE klass) {
    sfRectangleShape *shape = sfRectangleShape_create();
    VALUE rb_size;

    rb_scan_args(argc, argv, "01", &rb_size);

    if (!NIL_P(rb_size)) {
        sfRectangleShape_setSize(shape, vec2f_from_rb(rb_size));
    }

    return RectangleShape_wrap(klass, shape);
}

static VALUE RectangleShape_copy(VALUE self) {
    RectangleShape *ptr = Get_RectangleShape(self);
    VALUE copy = RectangleShape_wrap(Get_Klass_RectangleShape(), sfRectangleShape_copy(ptr->shape));

    if (!NIL_P(ptr->rb_texture)) {
        RectangleShape *copy_ptr = Get_RectangleShape(copy);

        copy_ptr->rb_texture = ptr->rb_texture;
        sfRectangleShape_setTexture(copy_ptr->shape, Get_Texture_Struct(ptr->rb_texture), false);
    }

    return copy;
}

static VALUE RectangleShape_get_size(VALUE self) {
    return vec2f_to_rb(sfRectangleShape_getSize(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_size(VALUE self, VALUE rb_size) {
    sfRectangleShape_setSize(Get_RectangleShape_Struct(self), vec2f_from_rb(rb_size));
    return rb_size;
}

static VALUE RectangleShape_get_position(VALUE self) {
    return vec2f_to_rb(sfRectangleShape_getPosition(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_position(VALUE self, VALUE rb_position) {
    sfRectangleShape_setPosition(Get_RectangleShape_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

static VALUE RectangleShape_get_rotation(VALUE self) {
    return DBL2NUM(sfRectangleShape_getRotation(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_rotation(VALUE self, VALUE rb_rotation) {
    sfRectangleShape_setRotation(Get_RectangleShape_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

static VALUE RectangleShape_get_scale(VALUE self) {
    return vec2f_to_rb(sfRectangleShape_getScale(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_scale(VALUE self, VALUE rb_scale) {
    sfRectangleShape_setScale(Get_RectangleShape_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

static VALUE RectangleShape_get_origin(VALUE self) {
    return vec2f_to_rb(sfRectangleShape_getOrigin(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_origin(VALUE self, VALUE rb_origin) {
    sfRectangleShape_setOrigin(Get_RectangleShape_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

static VALUE RectangleShape_move(VALUE self, VALUE rb_offset) {
    sfRectangleShape_move(Get_RectangleShape_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

static VALUE RectangleShape_rotate(VALUE self, VALUE rb_angle) {
    sfRectangleShape_rotate(Get_RectangleShape_Struct(self), NUM2DBL(rb_angle));
    return self;
}

static VALUE RectangleShape_scale(VALUE self, VALUE rb_factors) {
    sfRectangleShape_scale(Get_RectangleShape_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

static VALUE RectangleShape_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfRectangleShape_getTransform(Get_RectangleShape_Struct(self)).matrix);
}

static VALUE RectangleShape_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(sfRectangleShape_getInverseTransform(Get_RectangleShape_Struct(self)).matrix);
}

static VALUE RectangleShape_get_fill_color(VALUE self) {
    return color_to_rb(sfRectangleShape_getFillColor(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_fill_color(VALUE self, VALUE rb_color) {
    sfRectangleShape_setFillColor(Get_RectangleShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE RectangleShape_get_outline_color(VALUE self) {
    return color_to_rb(sfRectangleShape_getOutlineColor(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_outline_color(VALUE self, VALUE rb_color) {
    sfRectangleShape_setOutlineColor(Get_RectangleShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE RectangleShape_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfRectangleShape_getOutlineThickness(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfRectangleShape_setOutlineThickness(Get_RectangleShape_Struct(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

static VALUE RectangleShape_set_texture(VALUE self, VALUE rb_texture) {
    RectangleShape *ptr = Get_RectangleShape(self);
    bool reset_rect = false;

    if (NIL_P(rb_texture)) {
        ptr->rb_texture = Qnil;
        sfRectangleShape_setTexture(ptr->shape, NULL, reset_rect);
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    ptr->rb_texture = rb_texture;
    sfRectangleShape_setTexture(ptr->shape, Get_Texture_Struct(rb_texture), reset_rect);

    return rb_texture;
}

static VALUE RectangleShape_get_texture(VALUE self) {
    return Get_RectangleShape(self)->rb_texture;
}

static VALUE RectangleShape_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfRectangleShape_getTextureRect(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfRectangleShape_setTextureRect(Get_RectangleShape_Struct(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

static VALUE RectangleShape_get_point_count(VALUE self) {
    return SIZET2NUM(sfRectangleShape_getPointCount(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_get_point(VALUE self, VALUE rb_index) {
    return vec2f_to_rb(sfRectangleShape_getPoint(Get_RectangleShape_Struct(self), (size_t) NUM2SIZET(rb_index)));
}

static VALUE RectangleShape_get_geometric_center(VALUE self) {
    return vec2f_to_rb(sfRectangleShape_getGeometricCenter(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_get_local_bounds(VALUE self) {
    return rect_to_rb(sfRectangleShape_getLocalBounds(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_get_global_bounds(VALUE self) {
    return rect_to_rb(sfRectangleShape_getGlobalBounds(Get_RectangleShape_Struct(self)));
}

static VALUE RectangleShape_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawRectangleShape,
                sfRenderTexture_drawRectangleShape, Get_RectangleShape_Struct(self),
                Get_RenderState_Struct(rb_state));

    return Qnil;
}

void Init_RectangleShape(VALUE rb_module) {
    rb_cRectangleShape = rb_define_class_under(rb_module, "RectangleShape", rb_cObject);

    rb_include_module(rb_cRectangleShape, Get_Module_Drawable());

    rb_define_singleton_method(rb_cRectangleShape, "new", RectangleShape_new, -1);

    rb_define_method(rb_cRectangleShape, "copy", RectangleShape_copy, 0);

    rb_define_method(rb_cRectangleShape, "size", RectangleShape_get_size, 0);
    rb_define_method(rb_cRectangleShape, "position", RectangleShape_get_position, 0);
    rb_define_method(rb_cRectangleShape, "rotation", RectangleShape_get_rotation, 0);
    rb_define_method(rb_cRectangleShape, "scale", RectangleShape_get_scale, 0);
    rb_define_method(rb_cRectangleShape, "origin", RectangleShape_get_origin, 0);
    rb_define_method(rb_cRectangleShape, "fill_color", RectangleShape_get_fill_color, 0);
    rb_define_method(rb_cRectangleShape, "outline_color", RectangleShape_get_outline_color, 0);
    rb_define_method(rb_cRectangleShape, "outline_thickness", RectangleShape_get_outline_thickness, 0);
    rb_define_method(rb_cRectangleShape, "texture", RectangleShape_get_texture, 0);
    rb_define_method(rb_cRectangleShape, "texture_rect", RectangleShape_get_texture_rect, 0);
    rb_define_method(rb_cRectangleShape, "point_count", RectangleShape_get_point_count, 0);
    rb_define_method(rb_cRectangleShape, "point", RectangleShape_get_point, 1);
    rb_define_method(rb_cRectangleShape, "geometric_center", RectangleShape_get_geometric_center, 0);
    rb_define_method(rb_cRectangleShape, "local_bounds", RectangleShape_get_local_bounds, 0);
    rb_define_method(rb_cRectangleShape, "global_bounds", RectangleShape_get_global_bounds, 0);
    rb_define_method(rb_cRectangleShape, "transform", RectangleShape_get_transform, 0);
    rb_define_method(rb_cRectangleShape, "inverse_transform", RectangleShape_get_inverse_transform, 0);
    rb_define_method(rb_cRectangleShape, "matrix", RectangleShape_get_transform, 0);

    rb_define_method(rb_cRectangleShape, "size=", RectangleShape_set_size, 1);
    rb_define_method(rb_cRectangleShape, "position=", RectangleShape_set_position, 1);
    rb_define_method(rb_cRectangleShape, "rotation=", RectangleShape_set_rotation, 1);
    rb_define_method(rb_cRectangleShape, "scale=", RectangleShape_set_scale, 1);
    rb_define_method(rb_cRectangleShape, "origin=", RectangleShape_set_origin, 1);
    rb_define_method(rb_cRectangleShape, "fill_color=", RectangleShape_set_fill_color, 1);
    rb_define_method(rb_cRectangleShape, "outline_color=", RectangleShape_set_outline_color, 1);
    rb_define_method(rb_cRectangleShape, "outline_thickness=", RectangleShape_set_outline_thickness, 1);
    rb_define_method(rb_cRectangleShape, "texture=", RectangleShape_set_texture, 1);
    rb_define_method(rb_cRectangleShape, "texture_rect=", RectangleShape_set_texture_rect, 1);

    rb_define_method(rb_cRectangleShape, "move", RectangleShape_move, 1);
    rb_define_method(rb_cRectangleShape, "rotate", RectangleShape_rotate, 1);
    rb_define_method(rb_cRectangleShape, "scale!", RectangleShape_scale, 1);
    rb_define_method(rb_cRectangleShape, "draw", RectangleShape_draw, 2);
}

VALUE Get_Klass_RectangleShape(void) {
    return rb_cRectangleShape;
}

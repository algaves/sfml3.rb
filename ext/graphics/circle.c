#include "graphics/circle.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/drawable.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "core/exceptions.h"
#include "graphics/transform.h"
#include "graphics/transformable.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "graphics/texture.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/sfml.h"

typedef struct {
    sfCircleShape* shape;
    VALUE rb_texture;
} Circle;

static VALUE rb_cCircle;

static void Circle_mark(void* ptr) {
    rb_gc_mark(((Circle*)ptr)->rb_texture);
}

static void Circle_free(void* ptr) {
    sfCircleShape_destroy(((Circle*)ptr)->shape);
    free(ptr);
}

static const rb_data_type_t Circle_data_type = {
    .wrap_struct_name = "SFML::Circle",
    .function = {.dmark = Circle_mark, .dfree = Circle_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static Circle* Get_Circle(VALUE self) {
    Circle* ptr;
    TypedData_Get_Struct(self, Circle, &Circle_data_type, ptr);
    return ptr;
}

static sfCircleShape* Get_Circle_Shape(VALUE self) {
    return Get_Circle(self)->shape;
}

static VALUE Circle_wrap(VALUE klass, sfCircleShape* shape) {
    Circle* ptr;

    if (shape == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create circle shape");
    }

    ptr = malloc(sizeof(Circle));
    ptr->shape = shape;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &Circle_data_type, ptr);
}

static VALUE Circle_new(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_radius;
    sfCircleShape* shape;

    rb_scan_args(argc, argv, "01", &rb_radius);

    shape = sfCircleShape_create();

    if (!NIL_P(rb_radius)) {
        sfCircleShape_setRadius(shape, NUM2DBL(rb_radius));
    }

    return Circle_wrap(klass, shape);
}

static VALUE Circle_copy(VALUE self) {
    Circle* ptr = Get_Circle(self);
    VALUE copy = Circle_wrap(Get_Klass_Circle(), sfCircleShape_copy(ptr->shape));

    if (!NIL_P(ptr->rb_texture)) {
        Circle* copy_ptr = Get_Circle(copy);

        copy_ptr->rb_texture = ptr->rb_texture;
        sfCircleShape_setTexture(copy_ptr->shape, Get_Texture_Struct(ptr->rb_texture), false);
    }

    return copy;
}

static VALUE Circle_set_fill_color(VALUE self, VALUE rb_color) {
    sfCircleShape_setFillColor(Get_Circle_Shape(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE Circle_get_fill_color(VALUE self) {
    return color_to_rb(sfCircleShape_getFillColor(Get_Circle_Shape(self)));
}

static VALUE Circle_set_outline_color(VALUE self, VALUE rb_color) {
    sfCircleShape_setOutlineColor(Get_Circle_Shape(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE Circle_get_outline_color(VALUE self) {
    return color_to_rb(sfCircleShape_getOutlineColor(Get_Circle_Shape(self)));
}

static VALUE Circle_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfCircleShape_setOutlineThickness(Get_Circle_Shape(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

static VALUE Circle_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfCircleShape_getOutlineThickness(Get_Circle_Shape(self)));
}

static VALUE Circle_set_radius(VALUE self, VALUE rb_radius) {
    sfCircleShape_setRadius(Get_Circle_Shape(self), NUM2DBL(rb_radius));
    return rb_radius;
}

static VALUE Circle_set_position(VALUE self, VALUE rb_position) {
    sfCircleShape_setPosition(Get_Circle_Shape(self), vec2f_from_rb(rb_position));
    return rb_position;
}

static VALUE Circle_set_rotation(VALUE self, VALUE rb_angle) {
    sfCircleShape_setRotation(Get_Circle_Shape(self), NUM2DBL(rb_angle));
    return rb_angle;
}

static VALUE Circle_set_origin(VALUE self, VALUE rb_origin) {
    sfCircleShape_setOrigin(Get_Circle_Shape(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

static VALUE Circle_set_scale(VALUE self, VALUE rb_scale) {
    sfCircleShape_setScale(Get_Circle_Shape(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

static VALUE Circle_get_radius(VALUE self) {
    return DBL2NUM(sfCircleShape_getRadius(Get_Circle_Shape(self)));
}

static VALUE Circle_get_position(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getPosition(Get_Circle_Shape(self)));
}

static VALUE Circle_get_rotation(VALUE self) {
    return DBL2NUM(sfCircleShape_getRotation(Get_Circle_Shape(self)));
}

static VALUE Circle_get_scale(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getScale(Get_Circle_Shape(self)));
}

static VALUE Circle_get_origin(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getOrigin(Get_Circle_Shape(self)));
}

static VALUE Circle_move(VALUE self, VALUE rb_move) {
    sfCircleShape_move(Get_Circle_Shape(self), vec2f_from_rb(rb_move));
    return self;
}

static VALUE Circle_rotate(VALUE self, VALUE rb_angle) {
    sfCircleShape_rotate(Get_Circle_Shape(self), NUM2DBL(rb_angle));
    return self;
}

static VALUE Circle_scale(VALUE self, VALUE rb_scale) {
    sfCircleShape_scale(Get_Circle_Shape(self), vec2f_from_rb(rb_scale));
    return self;
}

static VALUE Circle_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfCircleShape_getTransform(Get_Circle_Shape(self)).matrix);
}

static VALUE Circle_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(
        sfCircleShape_getInverseTransform(Get_Circle_Shape(self)).matrix);
}

static VALUE Circle_set_texture(VALUE self, VALUE rb_texture) {
    Circle* ptr = Get_Circle(self);

    if (NIL_P(rb_texture)) {
        ptr->rb_texture = Qnil;
        sfCircleShape_setTexture(ptr->shape, NULL, false);
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    ptr->rb_texture = rb_texture;
    sfCircleShape_setTexture(ptr->shape, Get_Texture_Struct(rb_texture), false);

    return rb_texture;
}

static VALUE Circle_get_texture(VALUE self) {
    return Get_Circle(self)->rb_texture;
}

static VALUE Circle_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfCircleShape_getTextureRect(Get_Circle_Shape(self)));
}

static VALUE Circle_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfCircleShape_setTextureRect(Get_Circle_Shape(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

static VALUE Circle_get_point_count(VALUE self) {
    return SIZET2NUM(sfCircleShape_getPointCount(Get_Circle_Shape(self)));
}

/* The number of segments the circle is approximated with -- more for a smoother
   outline, fewer for a polygon (3 gives a triangle). */
static VALUE Circle_set_point_count(VALUE self, VALUE rb_count) {
    sfCircleShape_setPointCount(Get_Circle_Shape(self), NUM2SIZET(rb_count));

    return self;
}

static VALUE Circle_get_point(VALUE self, VALUE rb_index) {
    return vec2f_to_rb(sfCircleShape_getPoint(Get_Circle_Shape(self), (size_t)NUM2SIZET(rb_index)));
}

static VALUE Circle_get_geometric_center(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getGeometricCenter(Get_Circle_Shape(self)));
}

static VALUE Circle_get_local_bounds(VALUE self) {
    return rect_to_rb(sfCircleShape_getLocalBounds(Get_Circle_Shape(self)));
}

static VALUE Circle_get_global_bounds(VALUE self) {
    return rect_to_rb(sfCircleShape_getGlobalBounds(Get_Circle_Shape(self)));
}

static VALUE Circle_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawCircleShape,
                sfRenderTexture_drawCircleShape, Get_Circle_Shape(self),
                Get_RenderState_Struct(rb_state));

    return Qnil;
}

void Init_Circle(VALUE rb_module) {
    rb_cCircle = rb_define_class_under(rb_module, "Circle", rb_cObject);

    rb_include_module(rb_cCircle, Get_Module_Drawable());

    rb_define_singleton_method(rb_cCircle, "new", Circle_new, -1);

    // methods
    rb_define_method(rb_cCircle, "copy", Circle_copy, 0);
    rb_define_method(rb_cCircle, "move", Circle_move, 1);
    rb_define_method(rb_cCircle, "rotate", Circle_rotate, 1);
    rb_define_method(rb_cCircle, "scale!", Circle_scale, 1);
    rb_define_method(rb_cCircle, "draw", Circle_draw, 2);

    // setters
    rb_define_method(rb_cCircle, "radius=", Circle_set_radius, 1);
    rb_define_method(rb_cCircle, "position=", Circle_set_position, 1);
    rb_define_method(rb_cCircle, "rotation=", Circle_set_rotation, 1);
    rb_define_method(rb_cCircle, "scale=", Circle_set_scale, 1);
    rb_define_method(rb_cCircle, "origin=", Circle_set_origin, 1);
    rb_define_method(rb_cCircle, "fill_color=", Circle_set_fill_color, 1);
    rb_define_method(rb_cCircle, "outline_color=", Circle_set_outline_color, 1);
    rb_define_method(rb_cCircle, "outline_thickness=", Circle_set_outline_thickness, 1);
    rb_define_method(rb_cCircle, "texture=", Circle_set_texture, 1);
    rb_define_method(rb_cCircle, "texture_rect=", Circle_set_texture_rect, 1);

    // getters
    rb_define_method(rb_cCircle, "radius", Circle_get_radius, 0);
    rb_define_method(rb_cCircle, "position", Circle_get_position, 0);
    rb_define_method(rb_cCircle, "rotation", Circle_get_rotation, 0);
    rb_define_method(rb_cCircle, "scale", Circle_get_scale, 0);
    rb_define_method(rb_cCircle, "origin", Circle_get_origin, 0);
    rb_define_method(rb_cCircle, "fill_color", Circle_get_fill_color, 0);
    rb_define_method(rb_cCircle, "outline_color", Circle_get_outline_color, 0);
    rb_define_method(rb_cCircle, "outline_thickness", Circle_get_outline_thickness, 0);
    rb_define_method(rb_cCircle, "texture", Circle_get_texture, 0);
    rb_define_method(rb_cCircle, "texture_rect", Circle_get_texture_rect, 0);
    rb_define_method(rb_cCircle, "point_count", Circle_get_point_count, 0);
    rb_define_method(rb_cCircle, "point_count=", Circle_set_point_count, 1);
    rb_define_method(rb_cCircle, "point", Circle_get_point, 1);
    rb_define_method(rb_cCircle, "geometric_center", Circle_get_geometric_center, 0);
    rb_define_method(rb_cCircle, "local_bounds", Circle_get_local_bounds, 0);
    rb_define_method(rb_cCircle, "global_bounds", Circle_get_global_bounds, 0);
    rb_define_method(rb_cCircle, "transform", Circle_get_transform, 0);
    rb_define_method(rb_cCircle, "inverse_transform", Circle_get_inverse_transform, 0);
    rb_define_method(rb_cCircle, "matrix", Circle_get_transform, 0);
}

void* Get_Circle_Struct(VALUE self) {
    return Get_Circle_Shape(self);
}

VALUE Get_Klass_Circle() {
    return rb_cCircle;
}

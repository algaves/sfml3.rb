#include "graphics/shape.h"

#include <ruby.h>
#include <stdlib.h>

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
    sfShape *shape;
    VALUE rb_self;
    VALUE rb_texture;
} CustomShape;

static VALUE rb_cCustomShape;

static void CustomShape_mark(void *ptr) {
    CustomShape *shape = ptr;

    rb_gc_mark(shape->rb_self);
    rb_gc_mark(shape->rb_texture);
}

static void CustomShape_free(void *ptr) {
    sfShape_destroy(((CustomShape *) ptr)->shape);
    free(ptr);
}

static const rb_data_type_t CustomShape_data_type = {
    .wrap_struct_name = "SFML::Shape",
    .function = {.dmark = CustomShape_mark, .dfree = CustomShape_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static CustomShape *Get_CustomShape(VALUE self) {
    CustomShape *ptr;
    TypedData_Get_Struct(self, CustomShape, &CustomShape_data_type, ptr);
    return ptr;
}

static sfShape *Get_CustomShape_Struct(VALUE self) {
    return Get_CustomShape(self)->shape;
}

/* Point access is defined in Ruby, and these callbacks are reached through
   C++ (SFML), so exceptions are contained and reported as a degenerate
   shape instead of unwinding into foreign frames. */
static VALUE CustomShape_get_point_count_body(VALUE v) {
    CustomShape *shape = (CustomShape *) v;

    return rb_funcall(shape->rb_self, rb_intern("point_count"), 0);
}

static size_t CustomShape_get_point_count(void *userData) {
    CustomShape *shape = userData;
    int state = 0;
    VALUE count = rb_protect(CustomShape_get_point_count_body, (VALUE) shape, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return 0;
    }

    return (size_t) NUM2SIZET(count);
}

typedef struct {
    CustomShape *shape;
    size_t index;
} PointContext;

static VALUE CustomShape_get_point_body(VALUE v) {
    PointContext *ctx = (PointContext *) v;

    return rb_funcall(ctx->shape->rb_self, rb_intern("point"), 1, SIZET2NUM(ctx->index));
}

static sfVector2f CustomShape_get_point(size_t index, void *userData) {
    CustomShape *shape = userData;
    PointContext ctx = {.shape = shape, .index = index};
    int state = 0;
    VALUE point = rb_protect(CustomShape_get_point_body, (VALUE) &ctx, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return (sfVector2f) {0, 0};
    }

    return vec2f_from_rb(point);
}

static VALUE CustomShape_new(VALUE klass) {
    CustomShape *ptr = malloc(sizeof(CustomShape));
    VALUE self;

    ptr->rb_self = Qnil;
    ptr->rb_texture = Qnil;
    ptr->shape = sfShape_create(CustomShape_get_point_count, CustomShape_get_point, ptr);

    if (ptr->shape == NULL) {
        free(ptr);
        rb_raise(rb_eRuntimeError, "failed to create shape");
    }

    self = TypedData_Wrap_Struct(klass, &CustomShape_data_type, ptr);
    ptr->rb_self = self;

    return self;
}

static VALUE CustomShape_update(VALUE self) {
    sfShape_update(Get_CustomShape_Struct(self));
    return self;
}

static VALUE CustomShape_get_position(VALUE self) {
    return vec2f_to_rb(sfShape_getPosition(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_position(VALUE self, VALUE rb_position) {
    sfShape_setPosition(Get_CustomShape_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

static VALUE CustomShape_get_rotation(VALUE self) {
    return DBL2NUM(sfShape_getRotation(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_rotation(VALUE self, VALUE rb_rotation) {
    sfShape_setRotation(Get_CustomShape_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

static VALUE CustomShape_get_scale(VALUE self) {
    return vec2f_to_rb(sfShape_getScale(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_scale(VALUE self, VALUE rb_scale) {
    sfShape_setScale(Get_CustomShape_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

static VALUE CustomShape_get_origin(VALUE self) {
    return vec2f_to_rb(sfShape_getOrigin(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_origin(VALUE self, VALUE rb_origin) {
    sfShape_setOrigin(Get_CustomShape_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

static VALUE CustomShape_move(VALUE self, VALUE rb_offset) {
    sfShape_move(Get_CustomShape_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

static VALUE CustomShape_rotate(VALUE self, VALUE rb_angle) {
    sfShape_rotate(Get_CustomShape_Struct(self), NUM2DBL(rb_angle));
    return self;
}

static VALUE CustomShape_scale(VALUE self, VALUE rb_factors) {
    sfShape_scale(Get_CustomShape_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

static VALUE CustomShape_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfShape_getTransform(Get_CustomShape_Struct(self)).matrix);
}

static VALUE CustomShape_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(sfShape_getInverseTransform(Get_CustomShape_Struct(self)).matrix);
}

static VALUE CustomShape_get_fill_color(VALUE self) {
    return color_to_rb(sfShape_getFillColor(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_fill_color(VALUE self, VALUE rb_color) {
    sfShape_setFillColor(Get_CustomShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE CustomShape_get_outline_color(VALUE self) {
    return color_to_rb(sfShape_getOutlineColor(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_outline_color(VALUE self, VALUE rb_color) {
    sfShape_setOutlineColor(Get_CustomShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE CustomShape_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfShape_getOutlineThickness(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfShape_setOutlineThickness(Get_CustomShape_Struct(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

static VALUE CustomShape_set_texture(VALUE self, VALUE rb_texture) {
    CustomShape *ptr = Get_CustomShape(self);

    if (NIL_P(rb_texture)) {
        ptr->rb_texture = Qnil;
        sfShape_setTexture(ptr->shape, NULL, false);
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    ptr->rb_texture = rb_texture;
    sfShape_setTexture(ptr->shape, Get_Texture_Struct(rb_texture), false);

    return rb_texture;
}

static VALUE CustomShape_get_texture(VALUE self) {
    return Get_CustomShape(self)->rb_texture;
}

static VALUE CustomShape_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfShape_getTextureRect(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfShape_setTextureRect(Get_CustomShape_Struct(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

static VALUE CustomShape_get_geometric_center(VALUE self) {
    return vec2f_to_rb(sfShape_getGeometricCenter(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_get_local_bounds(VALUE self) {
    return rect_to_rb(sfShape_getLocalBounds(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_get_global_bounds(VALUE self) {
    return rect_to_rb(sfShape_getGlobalBounds(Get_CustomShape_Struct(self)));
}

static VALUE CustomShape_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawShape, sfRenderTexture_drawShape,
                Get_CustomShape_Struct(self), Get_RenderState_Struct(rb_state));

    return Qnil;
}

void Init_Shape(VALUE rb_module) {
    rb_cCustomShape = rb_define_class_under(rb_module, "Shape", rb_cObject);

    rb_include_module(rb_cCustomShape, Get_Module_Drawable());

    rb_define_singleton_method(rb_cCustomShape, "new", CustomShape_new, 0);

    rb_define_method(rb_cCustomShape, "update!", CustomShape_update, 0);
    rb_define_method(rb_cCustomShape, "position", CustomShape_get_position, 0);
    rb_define_method(rb_cCustomShape, "rotation", CustomShape_get_rotation, 0);
    rb_define_method(rb_cCustomShape, "scale", CustomShape_get_scale, 0);
    rb_define_method(rb_cCustomShape, "origin", CustomShape_get_origin, 0);
    rb_define_method(rb_cCustomShape, "fill_color", CustomShape_get_fill_color, 0);
    rb_define_method(rb_cCustomShape, "outline_color", CustomShape_get_outline_color, 0);
    rb_define_method(rb_cCustomShape, "outline_thickness", CustomShape_get_outline_thickness, 0);
    rb_define_method(rb_cCustomShape, "texture", CustomShape_get_texture, 0);
    rb_define_method(rb_cCustomShape, "texture_rect", CustomShape_get_texture_rect, 0);
    rb_define_method(rb_cCustomShape, "geometric_center", CustomShape_get_geometric_center, 0);
    rb_define_method(rb_cCustomShape, "local_bounds", CustomShape_get_local_bounds, 0);
    rb_define_method(rb_cCustomShape, "global_bounds", CustomShape_get_global_bounds, 0);
    rb_define_method(rb_cCustomShape, "transform", CustomShape_get_transform, 0);
    rb_define_method(rb_cCustomShape, "inverse_transform", CustomShape_get_inverse_transform, 0);
    rb_define_method(rb_cCustomShape, "matrix", CustomShape_get_transform, 0);

    rb_define_method(rb_cCustomShape, "position=", CustomShape_set_position, 1);
    rb_define_method(rb_cCustomShape, "rotation=", CustomShape_set_rotation, 1);
    rb_define_method(rb_cCustomShape, "scale=", CustomShape_set_scale, 1);
    rb_define_method(rb_cCustomShape, "origin=", CustomShape_set_origin, 1);
    rb_define_method(rb_cCustomShape, "fill_color=", CustomShape_set_fill_color, 1);
    rb_define_method(rb_cCustomShape, "outline_color=", CustomShape_set_outline_color, 1);
    rb_define_method(rb_cCustomShape, "outline_thickness=", CustomShape_set_outline_thickness, 1);
    rb_define_method(rb_cCustomShape, "texture=", CustomShape_set_texture, 1);
    rb_define_method(rb_cCustomShape, "texture_rect=", CustomShape_set_texture_rect, 1);

    rb_define_method(rb_cCustomShape, "move", CustomShape_move, 1);
    rb_define_method(rb_cCustomShape, "rotate", CustomShape_rotate, 1);
    rb_define_method(rb_cCustomShape, "scale!", CustomShape_scale, 1);
    rb_define_method(rb_cCustomShape, "draw", CustomShape_draw, 2);
}

VALUE Get_Klass_Shape(void) {
    return rb_cCustomShape;
}

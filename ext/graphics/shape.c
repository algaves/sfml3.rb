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
    sfShape* shape;
    VALUE rb_self;
    VALUE rb_texture;
} CustomShape;

static VALUE rb_cCustomShape;

static void CustomShape_mark(void* ptr) {
    CustomShape* shape = ptr;

    rb_gc_mark(shape->rb_self);
    rb_gc_mark(shape->rb_texture);
}

static void CustomShape_free(void* ptr) {
    sfShape_destroy(((CustomShape*)ptr)->shape);
    free(ptr);
}

static const rb_data_type_t CustomShape_data_type = {
    .wrap_struct_name = "SFML::Shape",
    .function = {.dmark = CustomShape_mark, .dfree = CustomShape_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static CustomShape* Get_CustomShape(VALUE self) {
    CustomShape* ptr;
    TypedData_Get_Struct(self, CustomShape, &CustomShape_data_type, ptr);
    return ptr;
}

static sfShape* Get_CustomShape_Struct(VALUE self) {
    return Get_CustomShape(self)->shape;
}

/* Point access is defined in Ruby, and these callbacks are reached through
   C++ (SFML), so exceptions are contained and reported as a degenerate
   shape instead of unwinding into foreign frames. */
static VALUE CustomShape_get_point_count_body(VALUE v) {
    CustomShape* shape = (CustomShape*)v;

    return rb_funcall(shape->rb_self, rb_intern("point_count"), 0);
}

static size_t CustomShape_get_point_count(void* userData) {
    CustomShape* shape = userData;
    int state = 0;
    VALUE count = rb_protect(CustomShape_get_point_count_body, (VALUE)shape, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return 0;
    }

    return (size_t)NUM2SIZET(count);
}

typedef struct {
    CustomShape* shape;
    size_t index;
} PointContext;

static VALUE CustomShape_get_point_body(VALUE v) {
    PointContext* ctx = (PointContext*)v;

    return rb_funcall(ctx->shape->rb_self, rb_intern("point"), 1, SIZET2NUM(ctx->index));
}

static sfVector2f CustomShape_get_point(size_t index, void* userData) {
    CustomShape* shape = userData;
    PointContext ctx = {.shape = shape, .index = index};
    int state = 0;
    VALUE point = rb_protect(CustomShape_get_point_body, (VALUE)&ctx, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return (sfVector2f){0, 0};
    }

    return vec2f_from_rb(point);
}

static VALUE CustomShape_alloc(VALUE klass) {
    CustomShape* ptr = malloc(sizeof(CustomShape));
    VALUE self;

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate shape");
    }

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

/* call-seq:
 *   Shape.new -> Shape
 *
 * A base class meant to be subclassed: override #point_count and #point(i)
 * in Ruby to define the shape's geometry, then call #update! whenever it
 * changes.
 *
 * @return [Shape]
 */
static VALUE CustomShape_initialize(VALUE self) {
    return self;
}

/* call-seq: update! -> self
 *
 * Recomputes the shape's geometry by calling back into the subclass's
 * #point_count and #point(i). Call this whenever the shape's points
 * change.
 *
 * @return [self]
 */
static VALUE CustomShape_update(VALUE self) {
    sfShape_update(Get_CustomShape_Struct(self));
    return self;
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2]
 */
static VALUE CustomShape_get_position(VALUE self) {
    return vec2f_to_rb(sfShape_getPosition(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   position=(value) -> Vector2
 *
 * Sets the object's position.
 *
 * @return [Vector2] +value+
 */
static VALUE CustomShape_set_position(VALUE self, VALUE rb_position) {
    sfShape_setPosition(Get_CustomShape_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

/* call-seq: rotation -> Float
 *
 * Returns the object's rotation, in degrees.
 *
 * @return [Float]
 */
static VALUE CustomShape_get_rotation(VALUE self) {
    return DBL2NUM(sfShape_getRotation(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   rotation=(value) -> Float
 *
 * Sets the object's rotation, in degrees.
 *
 * @return [Float] +value+
 */
static VALUE CustomShape_set_rotation(VALUE self, VALUE rb_rotation) {
    sfShape_setRotation(Get_CustomShape_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

/* call-seq: scale -> Vector2
 *
 * Returns the object's scale factors.
 *
 * @return [Vector2]
 */
static VALUE CustomShape_get_scale(VALUE self) {
    return vec2f_to_rb(sfShape_getScale(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   scale=(value) -> Vector2
 *
 * Sets the object's scale factors.
 *
 * @return [Vector2] +value+
 */
static VALUE CustomShape_set_scale(VALUE self, VALUE rb_scale) {
    sfShape_setScale(Get_CustomShape_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

/* call-seq: origin -> Vector2
 *
 * Returns the object's origin.
 *
 * @return [Vector2]
 */
static VALUE CustomShape_get_origin(VALUE self) {
    return vec2f_to_rb(sfShape_getOrigin(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   origin=(value) -> Vector2
 *
 * Sets the object's origin.
 *
 * @return [Vector2] +value+
 */
static VALUE CustomShape_set_origin(VALUE self, VALUE rb_origin) {
    sfShape_setOrigin(Get_CustomShape_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

/* call-seq: move(offset) -> self
 *
 * Moves the object by +offset+.
 *
 * @return [self]
 */
static VALUE CustomShape_move(VALUE self, VALUE rb_offset) {
    sfShape_move(Get_CustomShape_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

/* call-seq: rotate(angle) -> self
 *
 * Rotates the object by +angle+ degrees.
 *
 * @return [self]
 */
static VALUE CustomShape_rotate(VALUE self, VALUE rb_angle) {
    sfShape_rotate(Get_CustomShape_Struct(self), NUM2DBL(rb_angle));
    return self;
}

/* call-seq:
 *   scale!(factors) -> self
 *
 * Scales the object by +factors+ relative to its current scale.
 *
 * @return [self]
 */
static VALUE CustomShape_scale(VALUE self, VALUE rb_factors) {
    sfShape_scale(Get_CustomShape_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

/* call-seq: transform -> Array
 *
 * Also available as #matrix.
 *
 * @return [Array] the 3x3 row-major transform matrix
 */
static VALUE CustomShape_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfShape_getTransform(Get_CustomShape_Struct(self)).matrix);
}

/* call-seq: inverse_transform -> Array
 *
 * Returns the 3x3 row-major inverse of the object's transform matrix.
 *
 * @return [Array] the 3x3 row-major inverse transform matrix
 */
static VALUE CustomShape_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(
        sfShape_getInverseTransform(Get_CustomShape_Struct(self)).matrix);
}

/* call-seq: fill_color -> Color
 *
 * Returns the shape's fill color.
 *
 * @return [Color]
 */
static VALUE CustomShape_get_fill_color(VALUE self) {
    return color_to_rb(sfShape_getFillColor(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   fill_color=(value) -> Color
 *
 * Sets the shape's fill color.
 *
 * @return [Color] +value+
 */
static VALUE CustomShape_set_fill_color(VALUE self, VALUE rb_color) {
    sfShape_setFillColor(Get_CustomShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: outline_color -> Color
 *
 * Returns the shape's outline color.
 *
 * @return [Color]
 */
static VALUE CustomShape_get_outline_color(VALUE self) {
    return color_to_rb(sfShape_getOutlineColor(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   outline_color=(value) -> Color
 *
 * Sets the shape's outline color.
 *
 * @return [Color] +value+
 */
static VALUE CustomShape_set_outline_color(VALUE self, VALUE rb_color) {
    sfShape_setOutlineColor(Get_CustomShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: outline_thickness -> Float
 *
 * Returns the shape's outline thickness.
 *
 * @return [Float]
 */
static VALUE CustomShape_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfShape_getOutlineThickness(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   outline_thickness=(value) -> Float
 *
 * Sets the shape's outline thickness.
 *
 * @return [Float] +value+
 */
static VALUE CustomShape_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfShape_setOutlineThickness(Get_CustomShape_Struct(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

/* call-seq:
 *   texture=(value) -> Texture or nil
 *
 * Sets the object's texture.
 *
 * @return [Texture, nil] +value+
 */
static VALUE CustomShape_set_texture(VALUE self, VALUE rb_texture) {
    CustomShape* ptr = Get_CustomShape(self);

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

/* call-seq: texture -> Texture or nil
 *
 * Returns the object's texture, or +nil+ if it has none.
 *
 * @return [Texture, nil]
 */
static VALUE CustomShape_get_texture(VALUE self) {
    return Get_CustomShape(self)->rb_texture;
}

/* call-seq: texture_rect -> Rect
 *
 * Returns the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] the sub-rectangle of the texture displayed on the shape
 */
static VALUE CustomShape_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfShape_getTextureRect(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   texture_rect=(value) -> Rect
 *
 * Sets the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] +value+
 */
static VALUE CustomShape_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfShape_setTextureRect(Get_CustomShape_Struct(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

/* call-seq: geometric_center -> Vector2
 *
 * Returns the local position of the shape's geometric center.
 *
 * @return [Vector2] the local position of the shape's geometric center
 */
static VALUE CustomShape_get_geometric_center(VALUE self) {
    return vec2f_to_rb(sfShape_getGeometricCenter(Get_CustomShape_Struct(self)));
}

/* call-seq: local_bounds -> Rect
 *
 * Returns the bounding box in local (untransformed) coordinates.
 *
 * @return [Rect] the bounding box in local (untransformed) coordinates
 */
static VALUE CustomShape_get_local_bounds(VALUE self) {
    return rect_to_rb(sfShape_getLocalBounds(Get_CustomShape_Struct(self)));
}

/* call-seq: global_bounds -> Rect
 *
 * Returns the bounding box after the transform is applied.
 *
 * @return [Rect] the bounding box after transform is applied
 */
static VALUE CustomShape_get_global_bounds(VALUE self) {
    return rect_to_rb(sfShape_getGlobalBounds(Get_CustomShape_Struct(self)));
}

/* call-seq:
 *   draw(target, state) -> nil
 *
 * Draws the object onto +target+ using the given render +state+.
 *
 * @return [nil]
 */
static VALUE CustomShape_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    TargetView view = Get_RenderTarget_View(rb_target);

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(view, sfRenderWindow_drawShape, sfRenderTexture_drawShape,
                Get_CustomShape_Struct(self), Get_RenderState_Struct(rb_state));

    return Qnil;
}

/* Document-class: SFML::Shape
 * A base class for user-defined shapes. Subclass it and implement
 * #point_count and #point(index) in Ruby to describe the shape's geometry,
 * then call #update! after construction and whenever the points change.
 * Drawable, transformable and stylable like the other SFML shapes.
 * Includes Drawable.
 *
 * @!attribute position
 *   The object's position.
 *   @return [Vector2]
 * @!attribute rotation
 *   The object's rotation, in degrees.
 *   @return [Float]
 * @!attribute scale
 *   The object's scale factors.
 *   @return [Vector2]
 * @!attribute origin
 *   The object's origin.
 *   @return [Vector2]
 * @!attribute fill_color
 *   The object's fill color.
 *   @return [Color]
 * @!attribute outline_color
 *   The object's outline color.
 *   @return [Color]
 * @!attribute outline_thickness
 *   The object's outline thickness.
 *   @return [Float]
 * @!attribute texture
 *   The object's texture, or +nil+ if it has none.
 *   @return [Texture, nil]
 * @!attribute texture_rect
 *   The sub-rectangle of the texture displayed on the object.
 *   @return [Rect]
 */
void Init_Shape(VALUE rb_mSFML) {
    rb_cCustomShape = rb_define_class_under(rb_mSFML, "Shape", rb_cObject);

    rb_define_alloc_func(rb_cCustomShape, CustomShape_alloc);

    rb_include_module(rb_cCustomShape, Get_Module_Drawable());

    rb_define_method(rb_cCustomShape, "initialize", CustomShape_initialize, 0);

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

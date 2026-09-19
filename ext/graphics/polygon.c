#include "graphics/polygon.h"

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
    sfConvexShape* shape;
    VALUE rb_texture;
} ConvexShape;

static VALUE rb_cConvexShape;

static void ConvexShape_mark(void* ptr) {
    rb_gc_mark(((ConvexShape*)ptr)->rb_texture);
}

static void ConvexShape_free(void* ptr) {
    sfConvexShape_destroy(((ConvexShape*)ptr)->shape);
    free(ptr);
}

static const rb_data_type_t ConvexShape_data_type = {
    .wrap_struct_name = "SFML::ConvexShape",
    .function = {.dmark = ConvexShape_mark, .dfree = ConvexShape_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static ConvexShape* Get_ConvexShape(VALUE self) {
    ConvexShape* ptr;
    TypedData_Get_Struct(self, ConvexShape, &ConvexShape_data_type, ptr);
    return ptr;
}

static sfConvexShape* Get_ConvexShape_Struct(VALUE self) {
    return Get_ConvexShape(self)->shape;
}

static VALUE ConvexShape_wrap(VALUE klass, sfConvexShape* shape) {
    ConvexShape* ptr;

    if (shape == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create convex shape");
    }

    ptr = malloc(sizeof(ConvexShape));
    ptr->shape = shape;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &ConvexShape_data_type, ptr);
}

/* call-seq:
 *   ConvexShape.new(point_count = 0) -> ConvexShape
 *
 * Creates a convex shape with room for at least +point_count+ points.
 *
 * @return [ConvexShape]
 */
static VALUE ConvexShape_new(int argc, VALUE* argv, VALUE klass) {
    sfConvexShape* shape = sfConvexShape_create();
    VALUE rb_point_count;

    rb_scan_args(argc, argv, "01", &rb_point_count);

    if (!NIL_P(rb_point_count)) {
        sfConvexShape_setPointCount(shape, (size_t)NUM2SIZET(rb_point_count));
    }

    return ConvexShape_wrap(klass, shape);
}

/* call-seq: copy -> ConvexShape
 *
 * Returns a deep copy of the object.
 *
 * @return [ConvexShape] an independent copy, including its texture reference
 */
static VALUE ConvexShape_copy(VALUE self) {
    ConvexShape* ptr = Get_ConvexShape(self);
    VALUE copy = ConvexShape_wrap(Get_Klass_ConvexShape(), sfConvexShape_copy(ptr->shape));

    if (!NIL_P(ptr->rb_texture)) {
        ConvexShape* copy_ptr = Get_ConvexShape(copy);

        copy_ptr->rb_texture = ptr->rb_texture;
        sfConvexShape_setTexture(copy_ptr->shape, Get_Texture_Struct(ptr->rb_texture), false);
    }

    return copy;
}

/* call-seq: point_count -> Integer
 *
 * Returns the number of points composing the shape.
 *
 * @return [Integer]
 */
static VALUE ConvexShape_get_point_count(VALUE self) {
    return SIZET2NUM(sfConvexShape_getPointCount(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   point_count=(value) -> Integer
 *
 * Sets the number of points composing the shape.
 *
 * @return [Integer] +value+
 */
static VALUE ConvexShape_set_point_count(VALUE self, VALUE rb_count) {
    sfConvexShape_setPointCount(Get_ConvexShape_Struct(self), (size_t)NUM2SIZET(rb_count));
    return rb_count;
}

static size_t ConvexShape_check_index(VALUE self, VALUE rb_index) {
    void* shape = Get_ConvexShape_Struct(self);
    size_t index = (size_t)NUM2SIZET(rb_index);
    size_t count = sfConvexShape_getPointCount(shape);

    if (index >= count) {
        rb_raise(rb_eIndexError, "index %zu outside of point count %zu", index, count);
    }

    return index;
}

/* call-seq: point(index) -> Vector2
 *
 * Returns the local position of the point at +index+.
 *
 * @return [Vector2] the local position of the point at +index+
 * @raise [IndexError] if +index+ is out of range
 */
static VALUE ConvexShape_get_point(VALUE self, VALUE rb_index) {
    return vec2f_to_rb(sfConvexShape_getPoint(Get_ConvexShape_Struct(self),
                                              ConvexShape_check_index(self, rb_index)));
}

/* call-seq: set_point(index, point) -> Vector2
 *
 * Sets the position of the point at +index+.
 *
 * @return [Vector2] +point+
 * @raise [IndexError] if +index+ is out of range
 */
static VALUE ConvexShape_set_point(VALUE self, VALUE rb_index, VALUE rb_point) {
    sfConvexShape_setPoint(Get_ConvexShape_Struct(self), ConvexShape_check_index(self, rb_index),
                           vec2f_from_rb(rb_point));
    return rb_point;
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2]
 */
static VALUE ConvexShape_get_position(VALUE self) {
    return vec2f_to_rb(sfConvexShape_getPosition(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   position=(value) -> Vector2
 *
 * Sets the object's position.
 *
 * @return [Vector2] +value+
 */
static VALUE ConvexShape_set_position(VALUE self, VALUE rb_position) {
    sfConvexShape_setPosition(Get_ConvexShape_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

/* call-seq: rotation -> Float
 *
 * Returns the object's rotation, in degrees.
 *
 * @return [Float]
 */
static VALUE ConvexShape_get_rotation(VALUE self) {
    return DBL2NUM(sfConvexShape_getRotation(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   rotation=(value) -> Float
 *
 * Sets the object's rotation, in degrees.
 *
 * @return [Float] +value+
 */
static VALUE ConvexShape_set_rotation(VALUE self, VALUE rb_rotation) {
    sfConvexShape_setRotation(Get_ConvexShape_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

/* call-seq: scale -> Vector2
 *
 * Returns the object's scale factors.
 *
 * @return [Vector2]
 */
static VALUE ConvexShape_get_scale(VALUE self) {
    return vec2f_to_rb(sfConvexShape_getScale(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   scale=(value) -> Vector2
 *
 * Sets the object's scale factors.
 *
 * @return [Vector2] +value+
 */
static VALUE ConvexShape_set_scale(VALUE self, VALUE rb_scale) {
    sfConvexShape_setScale(Get_ConvexShape_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

/* call-seq: origin -> Vector2
 *
 * Returns the object's origin.
 *
 * @return [Vector2]
 */
static VALUE ConvexShape_get_origin(VALUE self) {
    return vec2f_to_rb(sfConvexShape_getOrigin(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   origin=(value) -> Vector2
 *
 * Sets the object's origin.
 *
 * @return [Vector2] +value+
 */
static VALUE ConvexShape_set_origin(VALUE self, VALUE rb_origin) {
    sfConvexShape_setOrigin(Get_ConvexShape_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

/* call-seq: move(offset) -> self
 *
 * Moves the object by +offset+.
 *
 * @return [self]
 */
static VALUE ConvexShape_move(VALUE self, VALUE rb_offset) {
    sfConvexShape_move(Get_ConvexShape_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

/* call-seq: rotate(angle) -> self
 *
 * Rotates the object by +angle+ degrees.
 *
 * @return [self]
 */
static VALUE ConvexShape_rotate(VALUE self, VALUE rb_angle) {
    sfConvexShape_rotate(Get_ConvexShape_Struct(self), NUM2DBL(rb_angle));
    return self;
}

/* call-seq:
 *   scale!(factors) -> self
 *
 * Scales the object by +factors+ relative to its current scale.
 *
 * @return [self]
 */
static VALUE ConvexShape_scale(VALUE self, VALUE rb_factors) {
    sfConvexShape_scale(Get_ConvexShape_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

/* call-seq: transform -> Array
 *
 * Also available as #matrix.
 *
 * @return [Array] the 3x3 row-major transform matrix
 */
static VALUE ConvexShape_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfConvexShape_getTransform(Get_ConvexShape_Struct(self)).matrix);
}

/* call-seq: inverse_transform -> Array
 *
 * Returns the 3x3 row-major inverse of the object's transform matrix.
 *
 * @return [Array] the 3x3 row-major inverse transform matrix
 */
static VALUE ConvexShape_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(
        sfConvexShape_getInverseTransform(Get_ConvexShape_Struct(self)).matrix);
}

/* call-seq: fill_color -> Color
 *
 * Returns the shape's fill color.
 *
 * @return [Color]
 */
static VALUE ConvexShape_get_fill_color(VALUE self) {
    return color_to_rb(sfConvexShape_getFillColor(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   fill_color=(value) -> Color
 *
 * Sets the shape's fill color.
 *
 * @return [Color] +value+
 */
static VALUE ConvexShape_set_fill_color(VALUE self, VALUE rb_color) {
    sfConvexShape_setFillColor(Get_ConvexShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: outline_color -> Color
 *
 * Returns the shape's outline color.
 *
 * @return [Color]
 */
static VALUE ConvexShape_get_outline_color(VALUE self) {
    return color_to_rb(sfConvexShape_getOutlineColor(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   outline_color=(value) -> Color
 *
 * Sets the shape's outline color.
 *
 * @return [Color] +value+
 */
static VALUE ConvexShape_set_outline_color(VALUE self, VALUE rb_color) {
    sfConvexShape_setOutlineColor(Get_ConvexShape_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: outline_thickness -> Float
 *
 * Returns the shape's outline thickness.
 *
 * @return [Float]
 */
static VALUE ConvexShape_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfConvexShape_getOutlineThickness(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   outline_thickness=(value) -> Float
 *
 * Sets the shape's outline thickness.
 *
 * @return [Float] +value+
 */
static VALUE ConvexShape_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfConvexShape_setOutlineThickness(Get_ConvexShape_Struct(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

/* call-seq:
 *   texture=(value) -> Texture or nil
 *
 * Sets the object's texture.
 *
 * @return [Texture, nil] +value+
 */
static VALUE ConvexShape_set_texture(VALUE self, VALUE rb_texture) {
    ConvexShape* ptr = Get_ConvexShape(self);

    if (NIL_P(rb_texture)) {
        ptr->rb_texture = Qnil;
        sfConvexShape_setTexture(ptr->shape, NULL, false);
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    ptr->rb_texture = rb_texture;
    sfConvexShape_setTexture(ptr->shape, Get_Texture_Struct(rb_texture), false);

    return rb_texture;
}

/* call-seq: texture -> Texture or nil
 *
 * Returns the object's texture, or +nil+ if it has none.
 *
 * @return [Texture, nil]
 */
static VALUE ConvexShape_get_texture(VALUE self) {
    return Get_ConvexShape(self)->rb_texture;
}

/* call-seq: texture_rect -> Rect
 *
 * Returns the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] the sub-rectangle of the texture displayed on the shape
 */
static VALUE ConvexShape_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfConvexShape_getTextureRect(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   texture_rect=(value) -> Rect
 *
 * Sets the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] +value+
 */
static VALUE ConvexShape_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfConvexShape_setTextureRect(Get_ConvexShape_Struct(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

/* call-seq: geometric_center -> Vector2
 *
 * Returns the local position of the shape's geometric center.
 *
 * @return [Vector2] the local position of the shape's geometric center
 */
static VALUE ConvexShape_get_geometric_center(VALUE self) {
    return vec2f_to_rb(sfConvexShape_getGeometricCenter(Get_ConvexShape_Struct(self)));
}

/* call-seq: local_bounds -> Rect
 *
 * Returns the bounding box in local (untransformed) coordinates.
 *
 * @return [Rect] the bounding box in local (untransformed) coordinates
 */
static VALUE ConvexShape_get_local_bounds(VALUE self) {
    return rect_to_rb(sfConvexShape_getLocalBounds(Get_ConvexShape_Struct(self)));
}

/* call-seq: global_bounds -> Rect
 *
 * Returns the bounding box after the transform is applied.
 *
 * @return [Rect] the bounding box after transform is applied
 */
static VALUE ConvexShape_get_global_bounds(VALUE self) {
    return rect_to_rb(sfConvexShape_getGlobalBounds(Get_ConvexShape_Struct(self)));
}

/* call-seq:
 *   draw(target, state) -> nil
 *
 * Draws the object onto +target+ using the given render +state+.
 *
 * @return [nil]
 */
static VALUE ConvexShape_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawConvexShape,
                sfRenderTexture_drawConvexShape, Get_ConvexShape_Struct(self),
                Get_RenderState_Struct(rb_state));

    return Qnil;
}

/* Document-class: SFML::ConvexShape
 * A convex polygon shape defined by an arbitrary set of points, drawable,
 * transformable and stylable like the other SFML shapes. Includes
 * Drawable.
 *
 * The polygon must remain convex; passing points that describe a concave
 * shape produces undefined rendering.
 *
 * @!attribute point_count
 *   The number of points composing the shape.
 *   @return [Integer]
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
void Init_ConvexShape(VALUE rb_mSFML) {
    rb_cConvexShape = rb_define_class_under(rb_mSFML, "ConvexShape", rb_cObject);

    rb_include_module(rb_cConvexShape, Get_Module_Drawable());

    rb_define_singleton_method(rb_cConvexShape, "new", ConvexShape_new, -1);

    rb_define_method(rb_cConvexShape, "copy", ConvexShape_copy, 0);

    rb_define_method(rb_cConvexShape, "point_count", ConvexShape_get_point_count, 0);
    rb_define_method(rb_cConvexShape, "point", ConvexShape_get_point, 1);
    rb_define_method(rb_cConvexShape, "position", ConvexShape_get_position, 0);
    rb_define_method(rb_cConvexShape, "rotation", ConvexShape_get_rotation, 0);
    rb_define_method(rb_cConvexShape, "scale", ConvexShape_get_scale, 0);
    rb_define_method(rb_cConvexShape, "origin", ConvexShape_get_origin, 0);
    rb_define_method(rb_cConvexShape, "fill_color", ConvexShape_get_fill_color, 0);
    rb_define_method(rb_cConvexShape, "outline_color", ConvexShape_get_outline_color, 0);
    rb_define_method(rb_cConvexShape, "outline_thickness", ConvexShape_get_outline_thickness, 0);
    rb_define_method(rb_cConvexShape, "texture", ConvexShape_get_texture, 0);
    rb_define_method(rb_cConvexShape, "texture_rect", ConvexShape_get_texture_rect, 0);
    rb_define_method(rb_cConvexShape, "geometric_center", ConvexShape_get_geometric_center, 0);
    rb_define_method(rb_cConvexShape, "local_bounds", ConvexShape_get_local_bounds, 0);
    rb_define_method(rb_cConvexShape, "global_bounds", ConvexShape_get_global_bounds, 0);
    rb_define_method(rb_cConvexShape, "transform", ConvexShape_get_transform, 0);
    rb_define_method(rb_cConvexShape, "inverse_transform", ConvexShape_get_inverse_transform, 0);
    rb_define_method(rb_cConvexShape, "matrix", ConvexShape_get_transform, 0);

    rb_define_method(rb_cConvexShape, "point_count=", ConvexShape_set_point_count, 1);
    rb_define_method(rb_cConvexShape, "set_point", ConvexShape_set_point, 2);
    rb_define_method(rb_cConvexShape, "position=", ConvexShape_set_position, 1);
    rb_define_method(rb_cConvexShape, "rotation=", ConvexShape_set_rotation, 1);
    rb_define_method(rb_cConvexShape, "scale=", ConvexShape_set_scale, 1);
    rb_define_method(rb_cConvexShape, "origin=", ConvexShape_set_origin, 1);
    rb_define_method(rb_cConvexShape, "fill_color=", ConvexShape_set_fill_color, 1);
    rb_define_method(rb_cConvexShape, "outline_color=", ConvexShape_set_outline_color, 1);
    rb_define_method(rb_cConvexShape, "outline_thickness=", ConvexShape_set_outline_thickness, 1);
    rb_define_method(rb_cConvexShape, "texture=", ConvexShape_set_texture, 1);
    rb_define_method(rb_cConvexShape, "texture_rect=", ConvexShape_set_texture_rect, 1);

    rb_define_method(rb_cConvexShape, "move", ConvexShape_move, 1);
    rb_define_method(rb_cConvexShape, "rotate", ConvexShape_rotate, 1);
    rb_define_method(rb_cConvexShape, "scale!", ConvexShape_scale, 1);
    rb_define_method(rb_cConvexShape, "draw", ConvexShape_draw, 2);
}

VALUE Get_Klass_ConvexShape(void) {
    return rb_cConvexShape;
}

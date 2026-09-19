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

static VALUE Circle_alloc(VALUE klass) {
    Circle* ptr = malloc(sizeof(Circle));
    sfCircleShape* shape;

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate circle shape");
    }

    shape = sfCircleShape_create();

    if (shape == NULL) {
        free(ptr);
        rb_raise(rb_eRuntimeError, "failed to create circle shape");
    }

    ptr->shape = shape;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &Circle_data_type, ptr);
}

/* call-seq:
 *   Circle.new(radius = 0) -> Circle
 *
 * Creates a circle shape with the given +radius+ (0 by default).
 *
 * @return [Circle]
 */
static VALUE Circle_initialize(int argc, VALUE* argv, VALUE self) {
    VALUE rb_radius;

    rb_scan_args(argc, argv, "01", &rb_radius);

    if (!NIL_P(rb_radius)) {
        sfCircleShape_setRadius(Get_Circle_Shape(self), NUM2DBL(rb_radius));
    }

    return self;
}

/* call-seq: copy -> Circle
 *
 * Returns a deep copy of the circle, including its texture reference.
 *
 * @return [Circle] an independent copy, including its texture reference
 */
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

/* call-seq:
 *   fill_color=(value) -> Color
 *
 * Sets the shape's fill color.
 *
 * @return [Color] +value+
 */
static VALUE Circle_set_fill_color(VALUE self, VALUE rb_color) {
    sfCircleShape_setFillColor(Get_Circle_Shape(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: fill_color -> Color
 *
 * Returns the shape's fill color.
 *
 * @return [Color]
 */
static VALUE Circle_get_fill_color(VALUE self) {
    return color_to_rb(sfCircleShape_getFillColor(Get_Circle_Shape(self)));
}

/* call-seq:
 *   outline_color=(value) -> Color
 *
 * Sets the shape's outline color.
 *
 * @return [Color] +value+
 */
static VALUE Circle_set_outline_color(VALUE self, VALUE rb_color) {
    sfCircleShape_setOutlineColor(Get_Circle_Shape(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: outline_color -> Color
 *
 * Returns the shape's outline color.
 *
 * @return [Color]
 */
static VALUE Circle_get_outline_color(VALUE self) {
    return color_to_rb(sfCircleShape_getOutlineColor(Get_Circle_Shape(self)));
}

/* call-seq:
 *   outline_thickness=(value) -> Float
 *
 * Sets the shape's outline thickness.
 *
 * @return [Float] +value+
 */
static VALUE Circle_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfCircleShape_setOutlineThickness(Get_Circle_Shape(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

/* call-seq: outline_thickness -> Float
 *
 * Returns the shape's outline thickness.
 *
 * @return [Float]
 */
static VALUE Circle_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfCircleShape_getOutlineThickness(Get_Circle_Shape(self)));
}

/* call-seq:
 *   radius=(value) -> Float
 *
 * Sets the circle's radius.
 *
 * @return [Float] +value+
 */
static VALUE Circle_set_radius(VALUE self, VALUE rb_radius) {
    sfCircleShape_setRadius(Get_Circle_Shape(self), NUM2DBL(rb_radius));
    return rb_radius;
}

/* call-seq:
 *   position=(value) -> Vector2
 *
 * Sets the object's position.
 *
 * @return [Vector2] +value+
 */
static VALUE Circle_set_position(VALUE self, VALUE rb_position) {
    sfCircleShape_setPosition(Get_Circle_Shape(self), vec2f_from_rb(rb_position));
    return rb_position;
}

/* call-seq:
 *   rotation=(value) -> Float
 *
 * Sets the object's rotation, in degrees.
 *
 * @return [Float] +value+
 */
static VALUE Circle_set_rotation(VALUE self, VALUE rb_angle) {
    sfCircleShape_setRotation(Get_Circle_Shape(self), NUM2DBL(rb_angle));
    return rb_angle;
}

/* call-seq:
 *   origin=(value) -> Vector2
 *
 * Sets the object's origin.
 *
 * @return [Vector2] +value+
 */
static VALUE Circle_set_origin(VALUE self, VALUE rb_origin) {
    sfCircleShape_setOrigin(Get_Circle_Shape(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

/* call-seq:
 *   scale=(value) -> Vector2
 *
 * Sets the object's scale factors.
 *
 * @return [Vector2] +value+
 */
static VALUE Circle_set_scale(VALUE self, VALUE rb_scale) {
    sfCircleShape_setScale(Get_Circle_Shape(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

/* call-seq: radius -> Float
 *
 * Returns the circle's radius.
 *
 * @return [Float]
 */
static VALUE Circle_get_radius(VALUE self) {
    return DBL2NUM(sfCircleShape_getRadius(Get_Circle_Shape(self)));
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2]
 */
static VALUE Circle_get_position(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getPosition(Get_Circle_Shape(self)));
}

/* call-seq: rotation -> Float
 *
 * Returns the object's rotation, in degrees.
 *
 * @return [Float]
 */
static VALUE Circle_get_rotation(VALUE self) {
    return DBL2NUM(sfCircleShape_getRotation(Get_Circle_Shape(self)));
}

/* call-seq: scale -> Vector2
 *
 * Returns the object's scale factors.
 *
 * @return [Vector2]
 */
static VALUE Circle_get_scale(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getScale(Get_Circle_Shape(self)));
}

/* call-seq: origin -> Vector2
 *
 * Returns the object's origin.
 *
 * @return [Vector2]
 */
static VALUE Circle_get_origin(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getOrigin(Get_Circle_Shape(self)));
}

/* call-seq: move(offset) -> self
 *
 * Moves the circle by +offset+.
 *
 * @return [self]
 */
static VALUE Circle_move(VALUE self, VALUE rb_move) {
    sfCircleShape_move(Get_Circle_Shape(self), vec2f_from_rb(rb_move));
    return self;
}

/* call-seq: rotate(angle) -> self
 *
 * Rotates the circle by +angle+ degrees.
 *
 * @return [self]
 */
static VALUE Circle_rotate(VALUE self, VALUE rb_angle) {
    sfCircleShape_rotate(Get_Circle_Shape(self), NUM2DBL(rb_angle));
    return self;
}

/* call-seq:
 *   scale!(factors) -> self
 *
 * Scales the circle by +factors+ relative to its current scale.
 *
 * @return [self]
 */
static VALUE Circle_scale(VALUE self, VALUE rb_scale) {
    sfCircleShape_scale(Get_Circle_Shape(self), vec2f_from_rb(rb_scale));
    return self;
}

/* call-seq: transform -> Array
 *
 * Also available as #matrix.
 *
 * @return [Array] the 3x3 row-major transform matrix
 */
static VALUE Circle_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfCircleShape_getTransform(Get_Circle_Shape(self)).matrix);
}

/* call-seq: inverse_transform -> Array
 *
 * Returns the 3x3 row-major inverse of the circle's transform matrix.
 *
 * @return [Array] the 3x3 row-major inverse transform matrix
 */
static VALUE Circle_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(
        sfCircleShape_getInverseTransform(Get_Circle_Shape(self)).matrix);
}

/* call-seq:
 *   texture=(value) -> Texture or nil
 *
 * Sets the object's texture.
 *
 * @return [Texture, nil] +value+
 */
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

/* call-seq: texture -> Texture or nil
 *
 * Returns the object's texture, or +nil+ if it has none.
 *
 * @return [Texture, nil]
 */
static VALUE Circle_get_texture(VALUE self) {
    return Get_Circle(self)->rb_texture;
}

/* call-seq: texture_rect -> Rect
 *
 * Returns the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] the sub-rectangle of the texture displayed on the circle
 */
static VALUE Circle_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfCircleShape_getTextureRect(Get_Circle_Shape(self)));
}

/* call-seq:
 *   texture_rect=(value) -> Rect
 *
 * Sets the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] +value+
 */
static VALUE Circle_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfCircleShape_setTextureRect(Get_Circle_Shape(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

/* call-seq: point_count -> Integer
 *
 * Returns the number of points composing the shape.
 *
 * @return [Integer] the number of points/segments the circle is
 *   approximated with
 */
static VALUE Circle_get_point_count(VALUE self) {
    return SIZET2NUM(sfCircleShape_getPointCount(Get_Circle_Shape(self)));
}

/* call-seq:
 *   point_count=(value) -> self
 *
 * The number of segments the circle is approximated with -- more for a
 * smoother outline, fewer for a polygon (3 gives a triangle).
 *
 * @return [self]
 */
static VALUE Circle_set_point_count(VALUE self, VALUE rb_count) {
    sfCircleShape_setPointCount(Get_Circle_Shape(self), NUM2SIZET(rb_count));

    return self;
}

/* call-seq: point(index) -> Vector2
 *
 * Returns the local position of the point at +index+.
 *
 * @return [Vector2] the local position of the point at +index+
 */
static VALUE Circle_get_point(VALUE self, VALUE rb_index) {
    return vec2f_to_rb(sfCircleShape_getPoint(Get_Circle_Shape(self), (size_t)NUM2SIZET(rb_index)));
}

/* call-seq: geometric_center -> Vector2
 *
 * Returns the local position of the shape's geometric center.
 *
 * @return [Vector2] the local position of the shape's geometric center
 */
static VALUE Circle_get_geometric_center(VALUE self) {
    return vec2f_to_rb(sfCircleShape_getGeometricCenter(Get_Circle_Shape(self)));
}

/* call-seq: local_bounds -> Rect
 *
 * Returns the bounding box in local (untransformed) coordinates.
 *
 * @return [Rect] the bounding box in local (untransformed) coordinates
 */
static VALUE Circle_get_local_bounds(VALUE self) {
    return rect_to_rb(sfCircleShape_getLocalBounds(Get_Circle_Shape(self)));
}

/* call-seq: global_bounds -> Rect
 *
 * Returns the bounding box after the transform is applied.
 *
 * @return [Rect] the bounding box after transform is applied
 */
static VALUE Circle_get_global_bounds(VALUE self) {
    return rect_to_rb(sfCircleShape_getGlobalBounds(Get_Circle_Shape(self)));
}

/* call-seq:
 *   draw(target, state) -> nil
 *
 * Draws the circle onto +target+ using the given render +state+.
 *
 * @return [nil]
 */
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

/* Document-class: SFML::Circle
 * A circle shape, drawable, transformable and stylable like the other
 * SFML shapes. Includes Drawable.
 *
 * @!attribute radius
 *   The circle's radius.
 *
 *   @return [Float]
 * @!attribute position
 *   The circle's position.
 *
 *   @return [Vector2]
 * @!attribute rotation
 *   The circle's rotation, in degrees.
 *
 *   @return [Float]
 * @!attribute scale
 *   The circle's scale factors.
 *
 *   @return [Vector2]
 * @!attribute origin
 *   The circle's origin, used as the center of rotation and scaling.
 *
 *   @return [Vector2]
 * @!attribute fill_color
 *   The circle's fill color.
 *
 *   @return [Color]
 * @!attribute outline_color
 *   The circle's outline color.
 *
 *   @return [Color]
 * @!attribute outline_thickness
 *   The circle's outline thickness.
 *
 *   @return [Float]
 * @!attribute texture
 *   The circle's texture, or +nil+ if it has none.
 *
 *   @return [Texture, nil]
 * @!attribute texture_rect
 *   The sub-rectangle of the texture displayed on the circle.
 *
 *   @return [Rect]
 * @!attribute point_count
 *   The number of segments the circle is approximated with.
 *
 *   @return [Integer] the number of segments the circle is approximated with
 */
void Init_Circle(VALUE rb_mSFML) {
    rb_cCircle = rb_define_class_under(rb_mSFML, "Circle", rb_cObject);

    rb_define_alloc_func(rb_cCircle, Circle_alloc);

    rb_include_module(rb_cCircle, Get_Module_Drawable());

    rb_define_method(rb_cCircle, "initialize", Circle_initialize, -1);

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

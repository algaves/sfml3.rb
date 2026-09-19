#include "graphics/transformable.h"

#include <ruby.h>
#include <stdio.h>

#include "graphics/transform.h"
#include "system/vec2.h"
#include "graphics/rect.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cTransformable;

static sfTransformable* Transformable_create() {
    return sfTransformable_create();
}

static void Transformable_free(void* ptr) {
    sfTransformable_destroy((sfTransformable*)ptr);
}

static const rb_data_type_t Transformable_data_type = {
    .wrap_struct_name = "SFML::Transformable",
    .function = {.dmark = NULL, .dfree = Transformable_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Transformable_alloc(VALUE klass) {
    sfTransformable* transformable = Transformable_create();

    if (transformable == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create transformable");
    }

    return TypedData_Wrap_Struct(klass, &Transformable_data_type, transformable);
}

/* call-seq:
 *   Transformable.new -> Transformable
 *
 * Creates a transformable object with a default position, rotation and scale.
 *
 * @return [Transformable]
 */
static VALUE Transformable_init(VALUE self) {
    return self;
}

/* call-seq:
 *   position=(value) -> self
 *
 * Sets the object's position.
 *
 * @return [self]
 */
static VALUE Transformable_set_position(VALUE self, VALUE rb_position) {
    sfVector2f position = VEC2_RB2C(rb_position);
    sfTransformable_setPosition(Get_Transformable_Struct(self), position);

    return self;
}

/* call-seq:
 *   rotation=(value) -> self
 *
 * Sets the object's rotation, in degrees.
 *
 * @return [self]
 */
static VALUE Transformable_set_rotation(VALUE self, VALUE rb_angle) {
    sfTransformable_setRotation(Get_Transformable_Struct(self), NUM2DBL(rb_angle));

    return self;
}

/* call-seq:
 *   scale=(value) -> self
 *
 * Sets the object's scale factors.
 *
 * @return [self]
 */
static VALUE Transformable_set_scale(VALUE self, VALUE rb_scale) {
    sfVector2f scale = VEC2_RB2C(rb_scale);
    sfTransformable_setScale(Get_Transformable_Struct(self), scale);

    return self;
}

/* call-seq:
 *   origin=(value) -> self
 *
 * Sets the object's origin.
 *
 * @return [self]
 */
static VALUE Transformable_set_origin(VALUE self, VALUE rb_origin) {
    sfVector2f origin = VEC2_RB2C(rb_origin);
    sfTransformable_setOrigin(Get_Transformable_Struct(self), origin);

    return self;
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2]
 */
static VALUE Transformable_get_position(VALUE self) {
    sfVector2f position = sfTransformable_getPosition(Get_Transformable_Struct(self));

    return VEC2_C2RB(position);
}

/* call-seq: rotation -> Float
 *
 * Returns the object's rotation, in degrees.
 *
 * @return [Float] the current rotation, in degrees
 */
static VALUE Transformable_get_rotation(VALUE self) {
    return DBL2NUM(sfTransformable_getRotation(Get_Transformable_Struct(self)));
}

/* call-seq: scale -> Vector2
 *
 * Returns the object's scale factors.
 *
 * @return [Vector2]
 */
static VALUE Transformable_get_scale(VALUE self) {
    sfVector2f scale = sfTransformable_getScale(Get_Transformable_Struct(self));

    return VEC2_C2RB(scale);
}

/* call-seq: origin -> Vector2
 *
 * Returns the object's origin.
 *
 * @return [Vector2]
 */
static VALUE Transformable_get_origin(VALUE self) {
    sfVector2f origin = sfTransformable_getOrigin(Get_Transformable_Struct(self));

    return VEC2_C2RB(origin);
}

/* call-seq:
 *   move(offset) -> self
 *
 * Adds +offset+ to the current position.
 *
 * @return [self]
 */
static VALUE Transformable_move(VALUE self, VALUE rb_move) {
    sfVector2f move = VEC2_RB2C(rb_move);
    sfTransformable_move(Get_Transformable_Struct(self), move);

    return self;
}

/* call-seq:
 *   rotate(angle) -> self
 *
 * Adds +angle+ degrees to the current rotation.
 *
 * @return [self]
 */
static VALUE Transformable_rotate(VALUE self, VALUE rb_angle) {
    sfTransformable_rotate(Get_Transformable_Struct(self), NUM2DBL(rb_angle));

    return self;
}

/* call-seq:
 *   scale!(factors) -> self
 *
 * Multiplies the current scale componentwise by +factors+.
 *
 * @return [self]
 */
static VALUE Transformable_scale(VALUE self, VALUE rb_scale) {
    sfVector2f scale = VEC2_RB2C(rb_scale);
    sfTransformable_scale(Get_Transformable_Struct(self), scale);

    return self;
}

/* call-seq: transform -> Array<Float>
 *
 * Returns the object's 3x3 row-major transform matrix.
 *
 * @return [Array<Float>] the 9-element matrix (also available as #matrix)
 *   combining this object's position, rotation, scale and origin
 */
static VALUE Transformable_get_matrix(VALUE self) {
    sfTransformable* transformable = Get_Transformable_Struct(self);
    sfTransform transform = sfTransformable_getTransform(transformable);

    return Transform_MatrixToArray(transform.matrix);
}

/* call-seq: inverse_transform -> Array<Float>
 *
 * Returns the 3x3 row-major inverse of the object's transform matrix.
 *
 * @return [Array<Float>] the inverse of #transform
 */
static VALUE Transformable_get_inverse_matrix(VALUE self) {
    sfTransform transform = sfTransformable_getInverseTransform(Get_Transformable_Struct(self));

    return Transform_MatrixToArray(transform.matrix);
}

/* call-seq: copy -> Transformable
 *
 * Returns a deep copy of the object.
 *
 * @return [Transformable] an independent copy
 */
static VALUE Transformable_copy(VALUE self) {
    sfTransformable* copy = sfTransformable_copy(Get_Transformable_Struct(self));
    VALUE other = TypedData_Wrap_Struct(rb_cTransformable, &Transformable_data_type, copy);

    rb_obj_call_init(other, 0, NULL);

    return other;
}

/* Document-class: SFML::Transformable
 * A standalone position/rotation/scale/origin, decoupled from any drawable
 * object -- the same transform state that Sprite, Text and Shape each carry
 * internally, usable on its own (e.g. as a scene-graph node).
 *
 * @!attribute position
 *   The object's position.
 *   @return [Vector2]
 * @!attribute rotation
 *   The object's rotation, in degrees.
 *   @return [Float] degrees
 * @!attribute scale
 *   The object's scale factors.
 *   @return [Vector2]
 * @!attribute origin
 *   The object's origin.
 *   @return [Vector2]
 */
void Init_Transformable(VALUE rb_mSFML) {
    rb_cTransformable = rb_define_class_under(rb_mSFML, "Transformable", rb_cObject);

    rb_define_alloc_func(rb_cTransformable, Transformable_alloc);
    rb_define_method(rb_cTransformable, "initialize", Transformable_init, 0);

    rb_define_method(rb_cTransformable, "position=", Transformable_set_position, 1);
    rb_define_method(rb_cTransformable, "rotation=", Transformable_set_rotation, 1);
    rb_define_method(rb_cTransformable, "scale=", Transformable_set_scale, 1);
    rb_define_method(rb_cTransformable, "origin=", Transformable_set_origin, 1);

    rb_define_method(rb_cTransformable, "position", Transformable_get_position, 0);
    rb_define_method(rb_cTransformable, "rotation", Transformable_get_rotation, 0);
    rb_define_method(rb_cTransformable, "scale", Transformable_get_scale, 0);
    rb_define_method(rb_cTransformable, "origin", Transformable_get_origin, 0);

    rb_define_method(rb_cTransformable, "move", Transformable_move, 1);
    rb_define_method(rb_cTransformable, "rotate", Transformable_rotate, 1);
    rb_define_method(rb_cTransformable, "scale!", Transformable_scale, 1);

    rb_define_method(rb_cTransformable, "transform", Transformable_get_matrix, 0);
    rb_define_method(rb_cTransformable, "matrix", Transformable_get_matrix, 0);
    rb_define_method(rb_cTransformable, "inverse_transform", Transformable_get_inverse_matrix, 0);

    rb_define_method(rb_cTransformable, "copy", Transformable_copy, 0);
}

void* Get_Transformable_Struct(VALUE self) {
    sfTransformable* transform;
    TypedData_Get_Struct(self, sfTransformable, &Transformable_data_type, transform);
    return transform;
}

VALUE Get_Klass_Transformable() {
    return rb_cTransformable;
}
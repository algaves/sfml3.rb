#include "graphics/transformable.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/circle.h"
#include "graphics/polygon.h"
#include "graphics/rect.h"
#include "graphics/rectangle.h"
#include "graphics/shape.h"
#include "graphics/sprite.h"
#include "graphics/text.h"
#include "graphics/transform.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/sfml.h"

/* SF::Graphics::Transformable is a module, because in Ruby a type can only have one
   superclass: every drawable mixes it in for the spatial state that sf::Sprite,
   sf::Text and the shapes each carry, while SF::Graphics::Drawable is the other mixin.
   The methods are implemented once here and dispatched to the receiver's
   concrete CSFML entry points, so Sprite, Text, Shape and every shape subclass
   share a single body instead of thirteen copies apiece.

   The standalone object is SF::Graphics::Transformable::Instance, an sfTransformable
   wrapper that includes the module, and Transformable.new returns one so the
   old `Transformable.new` / `class X < Transformable` usage keeps working. */
static VALUE rb_mTransformable;
static VALUE rb_cTransformableInstance;

typedef struct {
    sfTransformable* transformable;
} TransformableInstance;

static void TransformableInstance_free(void* ptr) {
    sfTransformable_destroy(((TransformableInstance*)ptr)->transformable);
    free(ptr);
}

static const rb_data_type_t TransformableInstance_data_type = {
    .wrap_struct_name = "SF::Graphics::Transformable::Instance",
    .function = {.dmark = NULL, .dfree = TransformableInstance_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static TransformableInstance* Get_TransformableInstance(VALUE self) {
    TransformableInstance* ptr;
    TypedData_Get_Struct(self, TransformableInstance, &TransformableInstance_data_type, ptr);
    return ptr;
}

static VALUE TransformableInstance_wrap(VALUE klass, sfTransformable* transformable) {
    TransformableInstance* ptr = malloc(sizeof(TransformableInstance));

    if (ptr == NULL) {
        sfTransformable_destroy(transformable);
        rb_raise(rb_eNoMemError, "failed to allocate transformable");
    }

    ptr->transformable = transformable;

    return TypedData_Wrap_Struct(klass, &TransformableInstance_data_type, ptr);
}

static VALUE TransformableInstance_alloc(VALUE klass) {
    sfTransformable* transformable = sfTransformable_create();

    if (transformable == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create transformable");
    }

    return TransformableInstance_wrap(klass, transformable);
}

void* Get_Transformable_Struct(VALUE self) {
    return Get_TransformableInstance(self)->transformable;
}

VALUE Get_Module_Transformable(void) {
    return rb_mTransformable;
}

/* Every wrapper that can act as a Transformable starts its typed data with its
   own handle, and the CSFML entry points are per type -- there is no common
   sfTransformable view of an sfSprite. Ordered most-derived first, so a
   CircleShape is dispatched to sfCircleShape_* rather than the sfShape_* it
   would otherwise match through its Shape superclass. */
typedef enum {
    TK_NONE,
    TK_INSTANCE,
    TK_SPRITE,
    TK_TEXT,
    TK_CIRCLE,
    TK_RECTANGLE,
    TK_CONVEX,
    TK_SHAPE
} TransformKind;

static TransformKind Transformable_kind(VALUE self) {
    if (rb_obj_is_kind_of(self, rb_cTransformableInstance)) {
        return TK_INSTANCE;
    }
    if (rb_obj_is_kind_of(self, Get_Klass_Sprite())) {
        return TK_SPRITE;
    }
    if (rb_obj_is_kind_of(self, Get_Klass_Text())) {
        return TK_TEXT;
    }
    if (rb_obj_is_kind_of(self, Get_Klass_CircleShape())) {
        return TK_CIRCLE;
    }
    if (rb_obj_is_kind_of(self, Get_Klass_RectangleShape())) {
        return TK_RECTANGLE;
    }
    if (rb_obj_is_kind_of(self, Get_Klass_ConvexShape())) {
        return TK_CONVEX;
    }
    if (rb_obj_is_kind_of(self, Get_Klass_Shape())) {
        return TK_SHAPE;
    }

    return TK_NONE;
}

/* CALL is a function-like macro of the form PREFIX_op(handle, ...): it is
   invoked once per candidate type with `prefix` bound to that type's CSFML
   prefix and `handle` to its typed handle, so every sf* call stays
   type-checked by the compiler. */
#define TF_DISPATCH(self, CALL)                                                                    \
    do {                                                                                           \
        switch (Transformable_kind(self)) {                                                        \
        case TK_INSTANCE: {                                                                        \
            sfTransformable* h = (sfTransformable*)Get_Transformable_Struct(self);                 \
            CALL(sfTransformable, h);                                                              \
            break;                                                                                 \
        }                                                                                          \
        case TK_SPRITE: {                                                                          \
            sfSprite* h = Get_Sprite_Struct(self);                                                 \
            CALL(sfSprite, h);                                                                     \
            break;                                                                                 \
        }                                                                                          \
        case TK_TEXT: {                                                                            \
            sfText* h = Get_Text_Struct(self);                                                     \
            CALL(sfText, h);                                                                       \
            break;                                                                                 \
        }                                                                                          \
        case TK_CIRCLE: {                                                                          \
            sfCircleShape* h = Get_CircleShape_Struct(self);                                       \
            CALL(sfCircleShape, h);                                                                \
            break;                                                                                 \
        }                                                                                          \
        case TK_RECTANGLE: {                                                                       \
            sfRectangleShape* h = Get_RectangleShape_Struct(self);                                 \
            CALL(sfRectangleShape, h);                                                             \
            break;                                                                                 \
        }                                                                                          \
        case TK_CONVEX: {                                                                          \
            sfConvexShape* h = Get_ConvexShape_Struct(self);                                       \
            CALL(sfConvexShape, h);                                                                \
            break;                                                                                 \
        }                                                                                          \
        case TK_SHAPE: {                                                                           \
            sfShape* h = Get_Shape_Struct(self);                                                   \
            CALL(sfShape, h);                                                                      \
            break;                                                                                 \
        }                                                                                          \
        default:                                                                                   \
            rb_raise(rb_eTypeError, "expected a Transformable");                                   \
        }                                                                                          \
    } while (0)

#define TF_SET_POSITION(prefix, h) prefix##_setPosition((h), position)
#define TF_GET_POSITION(prefix, h) (position = prefix##_getPosition(h))
#define TF_SET_ROTATION(prefix, h) prefix##_setRotation((h), NUM2DBL(rb_rotation))
#define TF_GET_ROTATION(prefix, h) (rotation = prefix##_getRotation(h))
#define TF_SET_SCALE(prefix, h) prefix##_setScale((h), scale)
#define TF_GET_SCALE(prefix, h) (scale = prefix##_getScale(h))
#define TF_SET_ORIGIN(prefix, h) prefix##_setOrigin((h), origin)
#define TF_GET_ORIGIN(prefix, h) (origin = prefix##_getOrigin(h))
#define TF_MOVE(prefix, h) prefix##_move((h), offset)
#define TF_ROTATE(prefix, h) prefix##_rotate((h), NUM2DBL(rb_angle))
#define TF_SCALE(prefix, h) prefix##_scale((h), factors)
#define TF_GET_TRANSFORM(prefix, h) (transform = prefix##_getTransform(h))
#define TF_GET_INVERSE_TRANSFORM(prefix, h) (transform = prefix##_getInverseTransform(h))

/* call-seq:
 *   position=(value) -> Vector2
 *
 * Sets the object's position.
 *
 * @return [Vector2] +value+
 */
static VALUE Transformable_set_position(VALUE self, VALUE rb_position) {
    sfVector2f position = vec2f_from_rb(rb_position);

    TF_DISPATCH(self, TF_SET_POSITION);

    return rb_position;
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2]
 */
static VALUE Transformable_get_position(VALUE self) {
    sfVector2f position;

    TF_DISPATCH(self, TF_GET_POSITION);

    return vec2f_to_rb(position);
}

/* call-seq:
 *   rotation=(value) -> Float
 *
 * Sets the object's rotation, in degrees.
 *
 * @return [Float] +value+
 */
static VALUE Transformable_set_rotation(VALUE self, VALUE rb_rotation) {
    TF_DISPATCH(self, TF_SET_ROTATION);

    return rb_rotation;
}

/* call-seq: rotation -> Float
 *
 * Returns the object's rotation, in degrees.
 *
 * @return [Float]
 */
static VALUE Transformable_get_rotation(VALUE self) {
    float rotation;

    TF_DISPATCH(self, TF_GET_ROTATION);

    return DBL2NUM(rotation);
}

/* call-seq:
 *   scale=(value) -> Vector2
 *
 * Sets the object's scale factors.
 *
 * @return [Vector2] +value+
 */
static VALUE Transformable_set_scale(VALUE self, VALUE rb_scale) {
    sfVector2f scale = vec2f_from_rb(rb_scale);

    TF_DISPATCH(self, TF_SET_SCALE);

    return rb_scale;
}

/* call-seq: scale -> Vector2
 *
 * Returns the object's scale factors.
 *
 * @return [Vector2]
 */
static VALUE Transformable_get_scale(VALUE self) {
    sfVector2f scale;

    TF_DISPATCH(self, TF_GET_SCALE);

    return vec2f_to_rb(scale);
}

/* call-seq:
 *   origin=(value) -> Vector2
 *
 * Sets the object's origin.
 *
 * @return [Vector2] +value+
 */
static VALUE Transformable_set_origin(VALUE self, VALUE rb_origin) {
    sfVector2f origin = vec2f_from_rb(rb_origin);

    TF_DISPATCH(self, TF_SET_ORIGIN);

    return rb_origin;
}

/* call-seq: origin -> Vector2
 *
 * Returns the object's origin.
 *
 * @return [Vector2]
 */
static VALUE Transformable_get_origin(VALUE self) {
    sfVector2f origin;

    TF_DISPATCH(self, TF_GET_ORIGIN);

    return vec2f_to_rb(origin);
}

/* call-seq:
 *   move(offset) -> self
 *
 * Adds +offset+ to the current position.
 *
 * @return [self]
 */
static VALUE Transformable_move(VALUE self, VALUE rb_move) {
    sfVector2f offset = vec2f_from_rb(rb_move);

    TF_DISPATCH(self, TF_MOVE);

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
    TF_DISPATCH(self, TF_ROTATE);

    return self;
}

/* call-seq:
 *   scale!(factors) -> self
 *
 * Multiplies the current scale componentwise by +factors+.
 *
 * @return [self]
 */
static VALUE Transformable_scale(VALUE self, VALUE rb_factors) {
    sfVector2f factors = vec2f_from_rb(rb_factors);

    TF_DISPATCH(self, TF_SCALE);

    return self;
}

/* call-seq: transform -> Array<Float>
 *
 * Returns the object's 3x3 row-major transform matrix.
 *
 * @return [Array<Float>] the 9-element matrix (also available as #matrix)
 */
static VALUE Transformable_get_transform(VALUE self) {
    sfTransform transform;

    TF_DISPATCH(self, TF_GET_TRANSFORM);

    return Transform_MatrixToArray(transform.matrix);
}

/* call-seq: inverse_transform -> Array<Float>
 *
 * Returns the 3x3 row-major inverse of the object's transform matrix.
 *
 * @return [Array<Float>] the inverse of #transform
 */
static VALUE Transformable_get_inverse_transform(VALUE self) {
    sfTransform transform;

    TF_DISPATCH(self, TF_GET_INVERSE_TRANSFORM);

    return Transform_MatrixToArray(transform.matrix);
}

/* call-seq:
 *   Transformable.new -> Transformable::Instance
 *
 * Creates a standalone transformable, independent of any drawable.
 *
 * @return [Transformable::Instance]
 */
static VALUE Transformable_s_new(int argc, VALUE* argv, VALUE klass) {
    (void)klass;

    return rb_class_new_instance(argc, argv, rb_cTransformableInstance);
}

/* call-seq: copy -> Transformable::Instance
 *
 * Returns a deep copy of the standalone transformable.
 *
 * @return [Transformable::Instance] an independent copy
 */
static VALUE TransformableInstance_copy(VALUE self) {
    TransformableInstance* ptr = Get_TransformableInstance(self);

    return TransformableInstance_wrap(rb_cTransformableInstance,
                                      sfTransformable_copy(ptr->transformable));
}

/* call-seq:
 *   Transformable::Instance.new -> Transformable::Instance
 *
 * The concrete standalone transformable returned by Transformable.new; use the
 * module instead of referring to this class directly.
 *
 * @return [Transformable::Instance]
 */
static VALUE TransformableInstance_initialize(VALUE self) {
    return self;
}

/* Document-class: SF::Graphics::Transformable::Instance
 * @private
 *
 * The concrete standalone transformable behind Transformable.new. It is an
 * implementation detail: mix SF::Graphics::Transformable into your own class, or call
 * Transformable.new, rather than referencing this class directly.
 */

/* Document-module: SF::Graphics::Transformable
 * A mixin providing the spatial state every drawable in SFML carries --
 * position, rotation, scale and origin, with the #move/#rotate/#scale!
 * mutators and the #transform/#inverse_transform matrices. Included by
 * SF::Graphics::Sprite, SF::Graphics::Text and SF::Graphics::Shape (and so by CircleShape,
 * RectangleShape and ConvexShape).
 *
 * The standalone SF::Graphics::Transformable::Instance (returned by Transformable.new) is
 * the same state decoupled from any drawable.
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
 *   The object's origin, the center of rotation and scaling.
 *   @return [Vector2]
 */
void Init_Transformable(VALUE rb_mGraphics) {
    rb_mTransformable = rb_define_module_under(rb_mGraphics, "Transformable");
    rb_cTransformableInstance = rb_define_class_under(rb_mTransformable, "Instance", rb_cObject);

    rb_include_module(rb_cTransformableInstance, rb_mTransformable);
    rb_define_alloc_func(rb_cTransformableInstance, TransformableInstance_alloc);
    rb_define_method(rb_cTransformableInstance, "initialize", TransformableInstance_initialize, 0);
    rb_define_method(rb_cTransformableInstance, "copy", TransformableInstance_copy, 0);

    rb_define_singleton_method(rb_mTransformable, "new", Transformable_s_new, -1);

    rb_define_method(rb_mTransformable, "position=", Transformable_set_position, 1);
    rb_define_method(rb_mTransformable, "rotation=", Transformable_set_rotation, 1);
    rb_define_method(rb_mTransformable, "scale=", Transformable_set_scale, 1);
    rb_define_method(rb_mTransformable, "origin=", Transformable_set_origin, 1);

    rb_define_method(rb_mTransformable, "position", Transformable_get_position, 0);
    rb_define_method(rb_mTransformable, "rotation", Transformable_get_rotation, 0);
    rb_define_method(rb_mTransformable, "scale", Transformable_get_scale, 0);
    rb_define_method(rb_mTransformable, "origin", Transformable_get_origin, 0);

    rb_define_method(rb_mTransformable, "move", Transformable_move, 1);
    rb_define_method(rb_mTransformable, "rotate", Transformable_rotate, 1);
    rb_define_method(rb_mTransformable, "scale!", Transformable_scale, 1);

    rb_define_method(rb_mTransformable, "transform", Transformable_get_transform, 0);
    rb_define_method(rb_mTransformable, "matrix", Transformable_get_transform, 0);
    rb_define_method(rb_mTransformable, "inverse_transform", Transformable_get_inverse_transform,
                     0);
}

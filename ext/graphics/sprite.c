#include "graphics/sprite.h"

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
    sfSprite* sprite;
    VALUE rb_texture;
} Sprite;

static VALUE rb_cSprite;

static void Sprite_mark(void* ptr) {
    rb_gc_mark(((Sprite*)ptr)->rb_texture);
}

static void Sprite_free(void* ptr) {
    if (((Sprite*)ptr)->sprite != NULL) {
        sfSprite_destroy(((Sprite*)ptr)->sprite);
    }

    free(ptr);
}

static const rb_data_type_t Sprite_data_type = {
    .wrap_struct_name = "SFML::Sprite",
    .function = {.dmark = Sprite_mark, .dfree = Sprite_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static Sprite* Get_Sprite(VALUE self) {
    Sprite* ptr;
    TypedData_Get_Struct(self, Sprite, &Sprite_data_type, ptr);
    return ptr;
}

static sfSprite* Get_Sprite_Struct(VALUE self) {
    return Get_Sprite(self)->sprite;
}

static VALUE Sprite_wrap(VALUE klass, sfSprite* sprite) {
    Sprite* ptr;

    if (sprite == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sprite");
    }

    ptr = malloc(sizeof(Sprite));
    ptr->sprite = sprite;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &Sprite_data_type, ptr);
}

static VALUE Sprite_initialize_copy(VALUE self, VALUE other) {
    (void)other;
    rb_raise(rb_eTypeError, "can't copy a %s", rb_obj_classname(self));
}

static VALUE Sprite_alloc(VALUE klass) {
    Sprite* ptr = malloc(sizeof(Sprite));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate sprite");
    }

    ptr->sprite = NULL;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &Sprite_data_type, ptr);
}

/* call-seq:
 *   Sprite.new(texture) -> Sprite
 *
 * CSFML requires a texture at creation time -- its C API unconditionally
 * dereferences a NULL texture argument rather than tolerating one, so unlike
 * most other constructors here, the texture is not optional.
 *
 * @return [Sprite]
 * @raise [ArgumentError] if +texture+ is not a Texture
 */
static VALUE Sprite_initialize(VALUE self, VALUE rb_texture) {
    Sprite* ptr;

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    TypedData_Get_Struct(self, Sprite, &Sprite_data_type, ptr);

    ptr->sprite = sfSprite_create(Get_Texture_Struct(rb_texture));
    ptr->rb_texture = rb_texture;

    if (ptr->sprite == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sprite");
    }

    return self;
}

/* call-seq: copy -> Sprite
 *
 * Returns a deep copy of the object.
 *
 * @return [Sprite] an independent copy
 */
static VALUE Sprite_copy(VALUE self) {
    Sprite* ptr = Get_Sprite(self);

    return Sprite_wrap(Get_Klass_Sprite(), sfSprite_copy(ptr->sprite));
}

/* call-seq:
 *   texture=(value) -> Texture or nil
 *
 * Sets the object's texture.
 *
 * @return [Texture, nil] +value+
 * @raise [TypeError] if +value+ is neither nil nor a Texture
 */
static VALUE Sprite_set_texture(VALUE self, VALUE rb_texture) {
    Sprite* ptr = Get_Sprite(self);

    if (NIL_P(rb_texture)) {
        ptr->rb_texture = Qnil;
        sfSprite_setTexture(ptr->sprite, NULL, false);
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    ptr->rb_texture = rb_texture;
    sfSprite_setTexture(ptr->sprite, Get_Texture_Struct(rb_texture), false);

    return rb_texture;
}

/* call-seq: texture -> Texture or nil
 *
 * Returns the object's texture, or +nil+ if it has none.
 *
 * @return [Texture, nil]
 */
static VALUE Sprite_get_texture(VALUE self) {
    return Get_Sprite(self)->rb_texture;
}

/* call-seq: texture_rect -> Rect
 *
 * Returns the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] the sub-rectangle of the texture that gets displayed
 */
static VALUE Sprite_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfSprite_getTextureRect(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   texture_rect=(value) -> Rect
 *
 * Sets the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] +value+
 */
static VALUE Sprite_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfSprite_setTextureRect(Get_Sprite_Struct(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

/* call-seq: color -> Color
 *
 * Returns the object's color.
 *
 * @return [Color] the tint color multiplied with the texture's pixels
 */
static VALUE Sprite_get_color(VALUE self) {
    return color_to_rb(sfSprite_getColor(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   color=(value) -> Color
 *
 * Sets the object's color.
 *
 * @return [Color] +value+
 */
static VALUE Sprite_set_color(VALUE self, VALUE rb_color) {
    sfSprite_setColor(Get_Sprite_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2]
 */
static VALUE Sprite_get_position(VALUE self) {
    return vec2f_to_rb(sfSprite_getPosition(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   position=(value) -> Vector2
 *
 * Sets the object's position.
 *
 * @return [Vector2] +value+
 */
static VALUE Sprite_set_position(VALUE self, VALUE rb_position) {
    sfSprite_setPosition(Get_Sprite_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

/* call-seq: rotation -> Float
 *
 * Returns the object's rotation, in degrees.
 *
 * @return [Float] degrees
 */
static VALUE Sprite_get_rotation(VALUE self) {
    return DBL2NUM(sfSprite_getRotation(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   rotation=(value) -> Float
 *
 * Sets the object's rotation, in degrees.
 *
 * @return [Float] +value+
 */
static VALUE Sprite_set_rotation(VALUE self, VALUE rb_rotation) {
    sfSprite_setRotation(Get_Sprite_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

/* call-seq: scale -> Vector2
 *
 * Returns the object's scale factors.
 *
 * @return [Vector2]
 */
static VALUE Sprite_get_scale(VALUE self) {
    return vec2f_to_rb(sfSprite_getScale(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   scale=(value) -> Vector2
 *
 * Sets the object's scale factors.
 *
 * @return [Vector2] +value+
 */
static VALUE Sprite_set_scale(VALUE self, VALUE rb_scale) {
    sfSprite_setScale(Get_Sprite_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

/* call-seq: origin -> Vector2
 *
 * Returns the object's origin.
 *
 * @return [Vector2]
 */
static VALUE Sprite_get_origin(VALUE self) {
    return vec2f_to_rb(sfSprite_getOrigin(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   origin=(value) -> Vector2
 *
 * Sets the object's origin.
 *
 * @return [Vector2] +value+
 */
static VALUE Sprite_set_origin(VALUE self, VALUE rb_origin) {
    sfSprite_setOrigin(Get_Sprite_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

/* call-seq:
 *   move(offset) -> self
 *
 * Moves the object by +offset+.
 *
 * @return [self]
 */
static VALUE Sprite_move(VALUE self, VALUE rb_offset) {
    sfSprite_move(Get_Sprite_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

/* call-seq:
 *   rotate(angle) -> self
 *
 * Rotates the object by +angle+ degrees.
 *
 * @return [self]
 */
static VALUE Sprite_rotate(VALUE self, VALUE rb_angle) {
    sfSprite_rotate(Get_Sprite_Struct(self), NUM2DBL(rb_angle));
    return self;
}

/* call-seq:
 *   scale!(factors) -> self
 *
 * Scales the object by +factors+ relative to its current scale.
 *
 * @return [self]
 */
static VALUE Sprite_scale(VALUE self, VALUE rb_factors) {
    sfSprite_scale(Get_Sprite_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

/* call-seq: transform -> Array<Float>
 *
 * Returns the object's 3x3 row-major transform matrix.
 *
 * @return [Array<Float>] the 9-element matrix (also available as #matrix)
 */
static VALUE Sprite_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfSprite_getTransform(Get_Sprite_Struct(self)).matrix);
}

/* call-seq: inverse_transform -> Array<Float>
 *
 * Returns the 3x3 row-major inverse of the object's transform matrix.
 *
 * @return [Array<Float>] the inverse of #transform
 */
static VALUE Sprite_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(sfSprite_getInverseTransform(Get_Sprite_Struct(self)).matrix);
}

/* call-seq: local_bounds -> Rect
 *
 * Returns the bounding box in local (untransformed) coordinates.
 *
 * @return [Rect] the bounding box in local coordinates, before any
 *   transform is applied
 */
static VALUE Sprite_get_local_bounds(VALUE self) {
    return rect_to_rb(sfSprite_getLocalBounds(Get_Sprite_Struct(self)));
}

/* call-seq: global_bounds -> Rect
 *
 * Returns the bounding box after the transform is applied.
 *
 * @return [Rect] the bounding box in the parent's coordinate system, after
 *   the current transform is applied
 */
static VALUE Sprite_get_global_bounds(VALUE self) {
    return rect_to_rb(sfSprite_getGlobalBounds(Get_Sprite_Struct(self)));
}

/* call-seq:
 *   draw(target, state) -> nil
 *
 * Part of the Drawable interface; call Target#draw instead of this
 * directly.
 *
 * @return [nil]
 * @raise [TypeError] if +target+ is not a Target or +state+ is not a
 *   RenderState
 */
static VALUE Sprite_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawSprite, sfRenderTexture_drawSprite,
                Get_Sprite_Struct(self), Get_RenderState_Struct(rb_state));

    return Qnil;
}

/* Document-class: SFML::Sprite
 * A drawable representation of a Texture (or a sub-rectangle of one),
 * positioned, rotated, scaled and tinted like any other Transformable
 * object.
 *
 * Includes +Drawable+.
 *
 * @!attribute texture
 *   The object's texture, or +nil+ if it has none.
 *   @return [Texture, nil]
 * @!attribute texture_rect
 *   The sub-rectangle of the texture displayed on the object.
 *   @return [Rect]
 * @!attribute color
 *   The object's color.
 *   @return [Color]
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
void Init_Sprite(VALUE rb_mSFML) {
    rb_cSprite = rb_define_class_under(rb_mSFML, "Sprite", rb_cObject);

    rb_define_alloc_func(rb_cSprite, Sprite_alloc);

    rb_include_module(rb_cSprite, Get_Module_Drawable());

    rb_define_method(rb_cSprite, "initialize", Sprite_initialize, 1);
    rb_define_private_method(rb_cSprite, "initialize_copy", Sprite_initialize_copy, 1);

    rb_define_method(rb_cSprite, "copy", Sprite_copy, 0);

    rb_define_method(rb_cSprite, "texture", Sprite_get_texture, 0);
    rb_define_method(rb_cSprite, "texture_rect", Sprite_get_texture_rect, 0);
    rb_define_method(rb_cSprite, "color", Sprite_get_color, 0);
    rb_define_method(rb_cSprite, "position", Sprite_get_position, 0);
    rb_define_method(rb_cSprite, "rotation", Sprite_get_rotation, 0);
    rb_define_method(rb_cSprite, "scale", Sprite_get_scale, 0);
    rb_define_method(rb_cSprite, "origin", Sprite_get_origin, 0);
    rb_define_method(rb_cSprite, "transform", Sprite_get_transform, 0);
    rb_define_method(rb_cSprite, "inverse_transform", Sprite_get_inverse_transform, 0);
    rb_define_method(rb_cSprite, "matrix", Sprite_get_transform, 0);
    rb_define_method(rb_cSprite, "local_bounds", Sprite_get_local_bounds, 0);
    rb_define_method(rb_cSprite, "global_bounds", Sprite_get_global_bounds, 0);

    rb_define_method(rb_cSprite, "texture=", Sprite_set_texture, 1);
    rb_define_method(rb_cSprite, "texture_rect=", Sprite_set_texture_rect, 1);
    rb_define_method(rb_cSprite, "color=", Sprite_set_color, 1);
    rb_define_method(rb_cSprite, "position=", Sprite_set_position, 1);
    rb_define_method(rb_cSprite, "rotation=", Sprite_set_rotation, 1);
    rb_define_method(rb_cSprite, "scale=", Sprite_set_scale, 1);
    rb_define_method(rb_cSprite, "origin=", Sprite_set_origin, 1);

    rb_define_method(rb_cSprite, "move", Sprite_move, 1);
    rb_define_method(rb_cSprite, "rotate", Sprite_rotate, 1);
    rb_define_method(rb_cSprite, "scale!", Sprite_scale, 1);
    rb_define_method(rb_cSprite, "draw", Sprite_draw, 2);
}

VALUE Get_Klass_Sprite(void) {
    return rb_cSprite;
}

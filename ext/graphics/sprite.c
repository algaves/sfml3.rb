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
    sfSprite *sprite;
    VALUE rb_texture;
} Sprite;

static VALUE rb_cSprite;

static void Sprite_mark(void *ptr) {
    rb_gc_mark(((Sprite *) ptr)->rb_texture);
}

static void Sprite_free(void *ptr) {
    sfSprite_destroy(((Sprite *) ptr)->sprite);
    free(ptr);
}

static const rb_data_type_t Sprite_data_type = {
    .wrap_struct_name = "SFML::Sprite",
    .function = {.dmark = Sprite_mark, .dfree = Sprite_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static Sprite *Get_Sprite(VALUE self) {
    Sprite *ptr;
    TypedData_Get_Struct(self, Sprite, &Sprite_data_type, ptr);
    return ptr;
}

static sfSprite *Get_Sprite_Struct(VALUE self) {
    return Get_Sprite(self)->sprite;
}

static VALUE Sprite_wrap(VALUE klass, sfSprite *sprite) {
    Sprite *ptr;

    if (sprite == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create sprite");
    }

    ptr = malloc(sizeof(Sprite));
    ptr->sprite = sprite;
    ptr->rb_texture = Qnil;

    return TypedData_Wrap_Struct(klass, &Sprite_data_type, ptr);
}

static VALUE Sprite_new(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_texture, self;
    Sprite *ptr;

    rb_scan_args(argc, argv, "01", &rb_texture);

    if (!NIL_P(rb_texture) && !rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    ptr = malloc(sizeof(Sprite));
    ptr->sprite = sfSprite_create(NIL_P(rb_texture) ? NULL : Get_Texture_Struct(rb_texture));
    ptr->rb_texture = NIL_P(rb_texture) ? Qnil : rb_texture;

    self = TypedData_Wrap_Struct(klass, &Sprite_data_type, ptr);

    return self;
}

static VALUE Sprite_copy(VALUE self) {
    Sprite *ptr = Get_Sprite(self);

    return Sprite_wrap(Get_Klass_Sprite(), sfSprite_copy(ptr->sprite));
}

static VALUE Sprite_set_texture(VALUE self, VALUE rb_texture) {
    Sprite *ptr = Get_Sprite(self);

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

static VALUE Sprite_get_texture(VALUE self) {
    return Get_Sprite(self)->rb_texture;
}

static VALUE Sprite_get_texture_rect(VALUE self) {
    return int_rect_to_rb(sfSprite_getTextureRect(Get_Sprite_Struct(self)));
}

static VALUE Sprite_set_texture_rect(VALUE self, VALUE rb_rect) {
    sfSprite_setTextureRect(Get_Sprite_Struct(self), int_rect_from_rb(rb_rect));
    return rb_rect;
}

static VALUE Sprite_get_color(VALUE self) {
    return color_to_rb(sfSprite_getColor(Get_Sprite_Struct(self)));
}

static VALUE Sprite_set_color(VALUE self, VALUE rb_color) {
    sfSprite_setColor(Get_Sprite_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE Sprite_get_position(VALUE self) {
    return vec2f_to_rb(sfSprite_getPosition(Get_Sprite_Struct(self)));
}

static VALUE Sprite_set_position(VALUE self, VALUE rb_position) {
    sfSprite_setPosition(Get_Sprite_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

static VALUE Sprite_get_rotation(VALUE self) {
    return DBL2NUM(sfSprite_getRotation(Get_Sprite_Struct(self)));
}

static VALUE Sprite_set_rotation(VALUE self, VALUE rb_rotation) {
    sfSprite_setRotation(Get_Sprite_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

static VALUE Sprite_get_scale(VALUE self) {
    return vec2f_to_rb(sfSprite_getScale(Get_Sprite_Struct(self)));
}

static VALUE Sprite_set_scale(VALUE self, VALUE rb_scale) {
    sfSprite_setScale(Get_Sprite_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

static VALUE Sprite_get_origin(VALUE self) {
    return vec2f_to_rb(sfSprite_getOrigin(Get_Sprite_Struct(self)));
}

static VALUE Sprite_set_origin(VALUE self, VALUE rb_origin) {
    sfSprite_setOrigin(Get_Sprite_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

static VALUE Sprite_move(VALUE self, VALUE rb_offset) {
    sfSprite_move(Get_Sprite_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

static VALUE Sprite_rotate(VALUE self, VALUE rb_angle) {
    sfSprite_rotate(Get_Sprite_Struct(self), NUM2DBL(rb_angle));
    return self;
}

static VALUE Sprite_scale(VALUE self, VALUE rb_factors) {
    sfSprite_scale(Get_Sprite_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

static VALUE Sprite_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfSprite_getTransform(Get_Sprite_Struct(self)).matrix);
}

static VALUE Sprite_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(sfSprite_getInverseTransform(Get_Sprite_Struct(self)).matrix);
}

static VALUE Sprite_get_local_bounds(VALUE self) {
    return rect_to_rb(sfSprite_getLocalBounds(Get_Sprite_Struct(self)));
}

static VALUE Sprite_get_global_bounds(VALUE self) {
    return rect_to_rb(sfSprite_getGlobalBounds(Get_Sprite_Struct(self)));
}

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

void Init_Sprite(VALUE rb_module) {
    rb_cSprite = rb_define_class_under(rb_module, "Sprite", rb_cObject);

    rb_include_module(rb_cSprite, Get_Module_Drawable());

    rb_define_singleton_method(rb_cSprite, "new", Sprite_new, -1);

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

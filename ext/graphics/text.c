#include "graphics/text.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/drawable.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/transform.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "graphics/font.h"
#include "graphics/enums.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/unicode.h"

typedef struct {
    sfText* text;
    VALUE rb_font;
} Text;

static VALUE rb_cText;

static void Text_mark(void* ptr) {
    rb_gc_mark(((Text*)ptr)->rb_font);
}

static void Text_free(void* ptr) {
    sfText_destroy(((Text*)ptr)->text);
    free(ptr);
}

static const rb_data_type_t Text_data_type = {
    .wrap_struct_name = "SFML::Text",
    .function = {.dmark = Text_mark, .dfree = Text_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static Text* Get_Text(VALUE self) {
    Text* ptr;
    TypedData_Get_Struct(self, Text, &Text_data_type, ptr);
    return ptr;
}

static sfText* Get_Text_Struct(VALUE self) {
    return Get_Text(self)->text;
}

static VALUE Text_wrap(VALUE klass, sfText* text) {
    Text* ptr;

    if (text == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create text");
    }

    ptr = malloc(sizeof(Text));
    ptr->text = text;
    ptr->rb_font = Qnil;

    return TypedData_Wrap_Struct(klass, &Text_data_type, ptr);
}

static VALUE Text_new(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_font, rb_string, rb_size, self;
    Text* ptr;

    rb_scan_args(argc, argv, "03", &rb_font, &rb_string, &rb_size);

    if (!NIL_P(rb_font) && !rb_obj_is_kind_of(rb_font, Get_Klass_Font())) {
        raise_invalid_argument_class(Get_Klass_Font());
    }

    ptr = malloc(sizeof(Text));
    ptr->text = sfText_create(NIL_P(rb_font) ? NULL : Get_Font_Struct(rb_font));
    ptr->rb_font = NIL_P(rb_font) ? Qnil : rb_font;

    if (ptr->text == NULL) {
        free(ptr);
        rb_raise(rb_eRuntimeError, "failed to create text");
    }

    self = TypedData_Wrap_Struct(klass, &Text_data_type, ptr);

    if (!NIL_P(rb_string)) {
        sfText_setString(ptr->text, StringValueCStr(rb_string));
    }

    if (!NIL_P(rb_size)) {
        sfText_setCharacterSize(ptr->text, (unsigned int)NUM2UINT(rb_size));
    }

    return self;
}

static VALUE Text_copy(VALUE self) {
    Text* ptr = Get_Text(self);
    VALUE copy = Text_wrap(Get_Klass_Text(), sfText_copy(ptr->text));
    Text* copy_ptr = Get_Text(copy);

    copy_ptr->rb_font = ptr->rb_font;

    return copy;
}

static VALUE Text_set_font(VALUE self, VALUE rb_font) {
    Text* ptr = Get_Text(self);

    if (NIL_P(rb_font)) {
        ptr->rb_font = Qnil;
        sfText_setFont(ptr->text, NULL);
        return rb_font;
    }

    if (!rb_obj_is_kind_of(rb_font, Get_Klass_Font())) {
        raise_invalid_argument_class(Get_Klass_Font());
    }

    ptr->rb_font = rb_font;
    sfText_setFont(ptr->text, Get_Font_Struct(rb_font));

    return rb_font;
}

static VALUE Text_get_font(VALUE self) {
    return Get_Text(self)->rb_font;
}

/* Through the UTF-32 entry points, not sfText_setString: that one decodes the
   bytes with the C locale and mangles anything outside ASCII. */
static VALUE Text_set_string(VALUE self, VALUE rb_string) {
    VALUE buffer = utf32_from_rb(rb_string);

    sfText_setUnicodeString(Get_Text_Struct(self), UTF32_PTR(buffer));

    RB_GC_GUARD(buffer);

    return rb_string;
}

static VALUE Text_get_string(VALUE self) {
    return utf32_to_rb(sfText_getUnicodeString(Get_Text_Struct(self)));
}

static VALUE Text_set_character_size(VALUE self, VALUE rb_size) {
    sfText_setCharacterSize(Get_Text_Struct(self), (unsigned int)NUM2UINT(rb_size));
    return rb_size;
}

static VALUE Text_get_character_size(VALUE self) {
    return UINT2NUM(sfText_getCharacterSize(Get_Text_Struct(self)));
}

static VALUE Text_set_style(VALUE self, VALUE rb_style) {
    sfText_setStyle(Get_Text_Struct(self), text_style_from_rb(rb_style));
    return rb_style;
}

static VALUE Text_get_style(VALUE self) {
    return text_style_to_rb(sfText_getStyle(Get_Text_Struct(self)));
}

static VALUE Text_get_fill_color(VALUE self) {
    return color_to_rb(sfText_getFillColor(Get_Text_Struct(self)));
}

static VALUE Text_set_fill_color(VALUE self, VALUE rb_color) {
    sfText_setFillColor(Get_Text_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE Text_get_outline_color(VALUE self) {
    return color_to_rb(sfText_getOutlineColor(Get_Text_Struct(self)));
}

static VALUE Text_set_outline_color(VALUE self, VALUE rb_color) {
    sfText_setOutlineColor(Get_Text_Struct(self), color_from_rb(rb_color));
    return rb_color;
}

static VALUE Text_get_outline_thickness(VALUE self) {
    return DBL2NUM(sfText_getOutlineThickness(Get_Text_Struct(self)));
}

static VALUE Text_set_outline_thickness(VALUE self, VALUE rb_thickness) {
    sfText_setOutlineThickness(Get_Text_Struct(self), NUM2DBL(rb_thickness));
    return rb_thickness;
}

static VALUE Text_get_letter_spacing(VALUE self) {
    return DBL2NUM(sfText_getLetterSpacing(Get_Text_Struct(self)));
}

static VALUE Text_set_letter_spacing(VALUE self, VALUE rb_spacing) {
    sfText_setLetterSpacing(Get_Text_Struct(self), NUM2DBL(rb_spacing));
    return rb_spacing;
}

static VALUE Text_get_line_spacing(VALUE self) {
    return DBL2NUM(sfText_getLineSpacing(Get_Text_Struct(self)));
}

static VALUE Text_set_line_spacing(VALUE self, VALUE rb_spacing) {
    sfText_setLineSpacing(Get_Text_Struct(self), NUM2DBL(rb_spacing));
    return rb_spacing;
}

static VALUE Text_get_position(VALUE self) {
    return vec2f_to_rb(sfText_getPosition(Get_Text_Struct(self)));
}

static VALUE Text_set_position(VALUE self, VALUE rb_position) {
    sfText_setPosition(Get_Text_Struct(self), vec2f_from_rb(rb_position));
    return rb_position;
}

static VALUE Text_get_rotation(VALUE self) {
    return DBL2NUM(sfText_getRotation(Get_Text_Struct(self)));
}

static VALUE Text_set_rotation(VALUE self, VALUE rb_rotation) {
    sfText_setRotation(Get_Text_Struct(self), NUM2DBL(rb_rotation));
    return rb_rotation;
}

static VALUE Text_get_scale(VALUE self) {
    return vec2f_to_rb(sfText_getScale(Get_Text_Struct(self)));
}

static VALUE Text_set_scale(VALUE self, VALUE rb_scale) {
    sfText_setScale(Get_Text_Struct(self), vec2f_from_rb(rb_scale));
    return rb_scale;
}

static VALUE Text_get_origin(VALUE self) {
    return vec2f_to_rb(sfText_getOrigin(Get_Text_Struct(self)));
}

static VALUE Text_set_origin(VALUE self, VALUE rb_origin) {
    sfText_setOrigin(Get_Text_Struct(self), vec2f_from_rb(rb_origin));
    return rb_origin;
}

static VALUE Text_move(VALUE self, VALUE rb_offset) {
    sfText_move(Get_Text_Struct(self), vec2f_from_rb(rb_offset));
    return self;
}

static VALUE Text_rotate(VALUE self, VALUE rb_angle) {
    sfText_rotate(Get_Text_Struct(self), NUM2DBL(rb_angle));
    return self;
}

static VALUE Text_scale(VALUE self, VALUE rb_factors) {
    sfText_scale(Get_Text_Struct(self), vec2f_from_rb(rb_factors));
    return self;
}

static VALUE Text_get_transform(VALUE self) {
    return Transform_MatrixToArray(sfText_getTransform(Get_Text_Struct(self)).matrix);
}

static VALUE Text_get_inverse_transform(VALUE self) {
    return Transform_MatrixToArray(sfText_getInverseTransform(Get_Text_Struct(self)).matrix);
}

static VALUE Text_find_character_pos(VALUE self, VALUE rb_index) {
    return vec2f_to_rb(sfText_findCharacterPos(Get_Text_Struct(self), (size_t)NUM2SIZET(rb_index)));
}

static VALUE Text_get_local_bounds(VALUE self) {
    return rect_to_rb(sfText_getLocalBounds(Get_Text_Struct(self)));
}

static VALUE Text_get_global_bounds(VALUE self) {
    return rect_to_rb(sfText_getGlobalBounds(Get_Text_Struct(self)));
}

static VALUE Text_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawText, sfRenderTexture_drawText,
                Get_Text_Struct(self), Get_RenderState_Struct(rb_state));

    return Qnil;
}

void Init_Text(VALUE rb_module) {
    rb_cText = rb_define_class_under(rb_module, "Text", rb_cObject);

    rb_include_module(rb_cText, Get_Module_Drawable());

    rb_define_singleton_method(rb_cText, "new", Text_new, -1);

    rb_define_method(rb_cText, "copy", Text_copy, 0);
    rb_define_method(rb_cText, "font", Text_get_font, 0);
    rb_define_method(rb_cText, "string", Text_get_string, 0);
    rb_define_method(rb_cText, "character_size", Text_get_character_size, 0);
    rb_define_method(rb_cText, "style", Text_get_style, 0);
    rb_define_method(rb_cText, "fill_color", Text_get_fill_color, 0);
    rb_define_method(rb_cText, "outline_color", Text_get_outline_color, 0);
    rb_define_method(rb_cText, "outline_thickness", Text_get_outline_thickness, 0);
    rb_define_method(rb_cText, "letter_spacing", Text_get_letter_spacing, 0);
    rb_define_method(rb_cText, "line_spacing", Text_get_line_spacing, 0);
    rb_define_method(rb_cText, "position", Text_get_position, 0);
    rb_define_method(rb_cText, "rotation", Text_get_rotation, 0);
    rb_define_method(rb_cText, "scale", Text_get_scale, 0);
    rb_define_method(rb_cText, "origin", Text_get_origin, 0);
    rb_define_method(rb_cText, "transform", Text_get_transform, 0);
    rb_define_method(rb_cText, "inverse_transform", Text_get_inverse_transform, 0);
    rb_define_method(rb_cText, "matrix", Text_get_transform, 0);
    rb_define_method(rb_cText, "find_character_pos", Text_find_character_pos, 1);
    rb_define_method(rb_cText, "local_bounds", Text_get_local_bounds, 0);
    rb_define_method(rb_cText, "global_bounds", Text_get_global_bounds, 0);

    rb_define_method(rb_cText, "font=", Text_set_font, 1);
    rb_define_method(rb_cText, "string=", Text_set_string, 1);
    rb_define_method(rb_cText, "character_size=", Text_set_character_size, 1);
    rb_define_method(rb_cText, "style=", Text_set_style, 1);
    rb_define_method(rb_cText, "fill_color=", Text_set_fill_color, 1);
    rb_define_method(rb_cText, "outline_color=", Text_set_outline_color, 1);
    rb_define_method(rb_cText, "outline_thickness=", Text_set_outline_thickness, 1);
    rb_define_method(rb_cText, "letter_spacing=", Text_set_letter_spacing, 1);
    rb_define_method(rb_cText, "line_spacing=", Text_set_line_spacing, 1);
    rb_define_method(rb_cText, "position=", Text_set_position, 1);
    rb_define_method(rb_cText, "rotation=", Text_set_rotation, 1);
    rb_define_method(rb_cText, "scale=", Text_set_scale, 1);
    rb_define_method(rb_cText, "origin=", Text_set_origin, 1);

    rb_define_method(rb_cText, "move", Text_move, 1);
    rb_define_method(rb_cText, "rotate", Text_rotate, 1);
    rb_define_method(rb_cText, "scale!", Text_scale, 1);
    rb_define_method(rb_cText, "draw", Text_draw, 2);
}

VALUE Get_Klass_Text(void) {
    return rb_cText;
}

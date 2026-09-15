#ifndef SFML_RB_GRAPHICS_COLOR_H
#define SFML_RB_GRAPHICS_COLOR_H

#include <ruby.h>

#include "core/sfml.h"

#define VALID_LENGTH_COLOR      4

#define UNPACK_COLOR(c_color) \
    (unsigned char) c_color.r, (unsigned char) c_color.g, (unsigned char) c_color.b, (unsigned char) c_color.a

#define COLOR_C2RB(c_color) color_to_rb(c_color)

#define COLOR_RB2C(rb_color) color_from_rb(rb_color)

void Init_Color(VALUE rb_module);

VALUE Get_Klass_Color(void);

void *Get_Color_Struct(VALUE self);

sfColor color_from_rb(VALUE rb_color);

VALUE color_to_rb(sfColor color);

VALUE color_new(int r, int g, int b, int a);

void color_check(VALUE rb_color);

void color_swap(sfColor *color_a, sfColor *color_b);

sfColor color_new_from_rb(VALUE rb_color);

VALUE color_new_from_c(sfColor color);

#endif //SFML_RB_GRAPHICS_COLOR_H

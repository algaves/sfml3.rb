#ifndef SFML_RB_GRAPHICS_RECT_H
#define SFML_RB_GRAPHICS_RECT_H

#include <ruby.h>

#include "core/sfml.h"

#define RECT_UNPACK(c_rect) (c_rect).position.x, (c_rect).position.y, (c_rect).size.x, (c_rect).size.y

#define RECT_C2RB(c_rect) rect_to_rb(c_rect)

#define RECT_RB2C(rb_rect) rect_from_rb(rb_rect)

void Init_Rect(VALUE rb_module);

VALUE Get_Klass_Rect(void);

void *Get_Rect_Struct(VALUE self);

sfFloatRect rect_from_rb(VALUE rb_rect);

VALUE rect_to_rb(sfFloatRect c_rect);

sfIntRect int_rect_from_rb(VALUE rb_rect);

VALUE int_rect_to_rb(sfIntRect c_rect);

void Rect_check(VALUE rb_rect);

#endif //SFML_RB_GRAPHICS_RECT_H

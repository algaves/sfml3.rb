#ifndef SFML_RB_GRAPHICS_CIRCLE_H
#define SFML_RB_GRAPHICS_CIRCLE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Circle(VALUE rb_module);

sfCircleShape* Get_CircleShape_Struct(VALUE self);

VALUE Get_Klass_CircleShape(void);

#endif // SFML_RB_GRAPHICS_CIRCLE_H

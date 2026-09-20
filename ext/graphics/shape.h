#ifndef SFML_RB_GRAPHICS_SHAPE_H
#define SFML_RB_GRAPHICS_SHAPE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Shape(VALUE rb_module);

VALUE Get_Klass_Shape(void);

sfShape* Get_Shape_Struct(VALUE self);

#endif // SFML_RB_GRAPHICS_SHAPE_H

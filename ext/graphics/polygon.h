#ifndef SFML_RB_GRAPHICS_POLYGON_H
#define SFML_RB_GRAPHICS_POLYGON_H

#include <ruby.h>

#include "core/sfml.h"

void Init_ConvexShape(VALUE rb_module);

VALUE Get_Klass_ConvexShape(void);

sfConvexShape* Get_ConvexShape_Struct(VALUE self);

#endif // SFML_RB_GRAPHICS_POLYGON_H

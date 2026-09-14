#ifndef SFML_RB_GRAPHICS_ENUMS_H
#define SFML_RB_GRAPHICS_ENUMS_H

#include <ruby.h>

#include "core/sfml.h"

const char *coordinate_type_name(sfCoordinateType type);

sfCoordinateType coordinate_type_from_rb(VALUE rb_type);

const char *primitive_type_name(sfPrimitiveType type);

sfPrimitiveType primitive_type_from_rb(VALUE rb_type);

VALUE text_style_to_rb(uint32_t style);

uint32_t text_style_from_rb(VALUE rb_style);

#endif //SFML_RB_GRAPHICS_ENUMS_H

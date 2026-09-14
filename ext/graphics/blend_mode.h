#ifndef SFML_RB_GRAPHICS_BLEND_MODE_H
#define SFML_RB_GRAPHICS_BLEND_MODE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_BlendMode(VALUE rb_module);

VALUE Get_Klass_BlendMode(void);

void *Get_BlendMode_Struct(VALUE self);

sfBlendMode blend_mode_from_rb(VALUE rb_mode);

VALUE blend_mode_to_rb(sfBlendMode mode);

#endif //SFML_RB_GRAPHICS_BLEND_MODE_H

#ifndef SFML_RB_GRAPHICS_STENCIL_MODE_H
#define SFML_RB_GRAPHICS_STENCIL_MODE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_StencilMode(VALUE rb_module);

VALUE Get_Klass_StencilMode(void);

void *Get_StencilMode_Struct(VALUE self);

sfStencilMode stencil_mode_from_rb(VALUE rb_mode);

VALUE stencil_mode_to_rb(sfStencilMode mode);

#endif //SFML_RB_GRAPHICS_STENCIL_MODE_H

#ifndef SFML_RB_GRAPHICS_SHADER_H
#define SFML_RB_GRAPHICS_SHADER_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Shader(VALUE rb_module);

VALUE Get_Klass_Shader(void);

const sfShader *Get_Shader_Struct(VALUE self);

#endif //SFML_RB_GRAPHICS_SHADER_H

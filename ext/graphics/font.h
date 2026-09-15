#ifndef SFML_RB_GRAPHICS_FONT_H
#define SFML_RB_GRAPHICS_FONT_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Font(VALUE rb_module);

VALUE Get_Klass_Font(void);

const sfFont *Get_Font_Struct(VALUE self);

#endif //SFML_RB_GRAPHICS_FONT_H

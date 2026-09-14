#ifndef SFML_RB_GRAPHICS_GLYPH_H
#define SFML_RB_GRAPHICS_GLYPH_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Glyph(VALUE rb_module);

VALUE Get_Klass_Glyph(void);

VALUE glyph_from_c(sfGlyph glyph);

#endif //SFML_RB_GRAPHICS_GLYPH_H

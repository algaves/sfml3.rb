#ifndef SFML_RB_GRAPHICS_TEXTURE_H
#define SFML_RB_GRAPHICS_TEXTURE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Texture(VALUE rb_module);

VALUE Get_Klass_Texture(void);

const sfTexture *Get_Texture_Struct(VALUE self);

VALUE texture_from_borrowed(const sfTexture *texture);

#endif //SFML_RB_GRAPHICS_TEXTURE_H

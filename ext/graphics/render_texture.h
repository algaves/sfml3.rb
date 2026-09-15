#ifndef SFML_RB_GRAPHICS_RENDER_TEXTURE_H
#define SFML_RB_GRAPHICS_RENDER_TEXTURE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_RenderTexture(VALUE rb_module);

VALUE Get_Klass_RenderTexture(void);

sfRenderTexture *Get_RenderTexture_Struct(VALUE self);

#endif //SFML_RB_GRAPHICS_RENDER_TEXTURE_H

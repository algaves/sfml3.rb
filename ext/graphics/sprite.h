#ifndef SFML_RB_GRAPHICS_SPRITE_H
#define SFML_RB_GRAPHICS_SPRITE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Sprite(VALUE rb_module);

VALUE Get_Klass_Sprite(void);

sfSprite* Get_Sprite_Struct(VALUE self);

#endif // SFML_RB_GRAPHICS_SPRITE_H

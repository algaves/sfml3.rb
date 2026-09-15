#ifndef SFML_RB_WINDOW_VIDEO_MODE_H
#define SFML_RB_WINDOW_VIDEO_MODE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_VideoMode(VALUE rb_module);

sfVideoMode *Get_Mode_Struct(VALUE self);

VALUE Get_Klass_Mode();

#endif //SFML_RB_WINDOW_VIDEO_MODE_H

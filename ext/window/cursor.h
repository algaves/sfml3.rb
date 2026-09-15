#ifndef SFML_RB_WINDOW_CURSOR_H
#define SFML_RB_WINDOW_CURSOR_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Cursor(VALUE rb_module);

VALUE Get_Klass_Cursor(void);

const sfCursor *Get_Cursor_Struct(VALUE self);

#endif //SFML_RB_WINDOW_CURSOR_H

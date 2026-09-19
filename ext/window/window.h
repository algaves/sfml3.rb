#ifndef SFML_RB_WINDOW_WINDOW_H
#define SFML_RB_WINDOW_WINDOW_H

#include <ruby.h>

#include "window/window_base.h"

void Init_Window(VALUE rb_module);

VALUE Get_Klass_Window(void);

#endif // SFML_RB_WINDOW_WINDOW_H

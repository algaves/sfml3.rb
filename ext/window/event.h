#ifndef SFML_RB_WINDOW_EVENT_H
#define SFML_RB_WINDOW_EVENT_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Event(VALUE rb_module);

sfEvent *Get_Event_Struct(VALUE self);

VALUE Get_Klass_Event();

#endif //SFML_RB_WINDOW_EVENT_H

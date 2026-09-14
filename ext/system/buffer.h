#ifndef SFML_RB_SYSTEM_BUFFER_H
#define SFML_RB_SYSTEM_BUFFER_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Buffer(VALUE rb_module);

VALUE Get_Klass_Buffer(void);

void *Get_Buffer_Struct(VALUE self);

#endif //SFML_RB_SYSTEM_BUFFER_H

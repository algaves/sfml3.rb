#ifndef SFML_RB_SYSTEM_TIME_H
#define SFML_RB_SYSTEM_TIME_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Time(VALUE rb_module);

VALUE Get_Klass_Time(void);

void *Get_Time_Struct(VALUE self);

sfTime time_from_rb(VALUE rb_time);

VALUE time_to_rb(sfTime c_time);

#endif //SFML_RB_SYSTEM_TIME_H

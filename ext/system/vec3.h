#ifndef SFML_RB_SYSTEM_VEC3_H
#define SFML_RB_SYSTEM_VEC3_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Vector3(VALUE rb_module);

VALUE Get_Klass_Vector3(void);

void *Get_Vector3_Struct(VALUE self);

sfVector3f vec3f_from_rb(VALUE rb_vec);

VALUE vec3f_to_rb(sfVector3f c_vec);

#endif //SFML_RB_SYSTEM_VEC3_H

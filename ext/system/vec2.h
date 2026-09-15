#ifndef SFML_RB_SYSTEM_VEC2_H
#define SFML_RB_SYSTEM_VEC2_H

#include <ruby.h>

#include "core/sfml.h"

#define UNPACK_VEC2(c_type, c_vec) (c_type) c_vec.x, (c_type) c_vec.y

#define VEC2_C2RB(c_vec) vec2f_to_rb((sfVector2f) { (float) (c_vec).x, (float) (c_vec).y })

#define VEC2_C2RB_T(c_type, c_vec) \
    vec2f_to_rb((sfVector2f) { (float) (c_type) (c_vec).x, (float) (c_type) (c_vec).y })

#define VEC2_RB2C(rb_vec) vec2f_from_rb(rb_vec)

#define VEC2_RB2C_T(c_type, rb_vec) \
    (c_type) NUM2DBL(rb_ary_entry(rb_vec, 0)), (c_type) NUM2DBL(rb_ary_entry(rb_vec, 1))

#define VEC2_SET_X(rb_vec, x) rb_ary_store(rb_vec, 0, DBL2NUM(x))

#define VEC2_SET_Y(rb_vec, y) rb_ary_store(rb_vec, 1, DBL2NUM(y))

#define VEC2_GET_X(rb_vec) NUM2DBL(rb_ary_entry(rb_vec, 0))

#define VEC2_GET_Y(rb_vec) NUM2DBL(rb_ary_entry(rb_vec, 1))

void Init_Vector2(VALUE rb_module);

VALUE Get_Klass_Vector2(void);

void *Get_Vector2_Struct(VALUE self);

sfVector2f vec2f_from_rb(VALUE rb_vec);

sfVector2i vec2i_from_rb(VALUE rb_vec);

sfVector2u vec2u_from_rb(VALUE rb_vec);

VALUE vec2f_to_rb(sfVector2f c_vec);

/* Backwards-compatible internal helpers. */
VALUE vec2_new(float x, float y);

void vec2_check(VALUE rb_arr);

VALUE vec2f_new_from_c(sfVector2f c_vec);

sfVector2i vec2i_new_from_ruby(VALUE rb_arr);

sfVector2u vec2u_new_from_ruby(VALUE rb_arr);

sfVector2f vec2f_new_from_ruby(VALUE rb_arr);

#endif //SFML_RB_SYSTEM_VEC2_H

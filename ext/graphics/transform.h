#ifndef SFML_RB_GRAPHICS_TRANSFORM_H
#define SFML_RB_GRAPHICS_TRANSFORM_H

#include <ruby.h>

#include "core/sfml.h"

#define MATRIX_LENGTH       9   // 3x3

void Init_Transform(VALUE rb_module);

VALUE Transform_MatrixToArray(float *c_matrix);

void Transform_ArrayToMatrix(VALUE rb_matrix, float *c_matrix);

void Transform_SwapMatrix(const float *c_matrix_a, float *c_matrix_b);

#endif //SFML_RB_GRAPHICS_TRANSFORM_H

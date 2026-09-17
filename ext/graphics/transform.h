#ifndef SFML_RB_GRAPHICS_TRANSFORM_H
#define SFML_RB_GRAPHICS_TRANSFORM_H

#include <ruby.h>

#include "core/sfml.h"

/* The 3x3 affine matrix SFML stores in sfTransform. Distinct from the 16-float
   4x4 OpenGL matrix that sfTransform_getMatrix() fills -- see Transform#gl_matrix. */
#define MATRIX_LENGTH 9 // 3x3

/* The 4x4 column-major matrix sfTransform_getMatrix() writes, for glLoadMatrixf. */
#define GL_MATRIX_LENGTH 16 // 4x4

void Init_Transform(VALUE rb_module);

VALUE Get_Klass_Transform(void);

/* Every class that exposes #transform / #inverse_transform returns the 3x3 as a
   plain Array through this, which is why those getters are Arrays and not
   Transform instances. */
VALUE Transform_MatrixToArray(float* c_matrix);

void Transform_ArrayToMatrix(VALUE rb_matrix, float* c_matrix);

/* Accepts either a Transform instance or a 9-element Array, so anything that
   takes a transform from the caller accepts both forms. */
sfTransform Transform_ArrayToTransform(VALUE rb_matrix);

VALUE Transform_TransformToArray(sfTransform c_transform);

/* Wraps a transform by value into a new Transform instance. */
VALUE Transform_wrap(sfTransform c_transform);

#endif // SFML_RB_GRAPHICS_TRANSFORM_H

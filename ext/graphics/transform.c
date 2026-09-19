#include "graphics/transform.h"

#include <ruby.h>
#include <stdio.h>

#include "system/vec2.h"
#include "graphics/rect.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cTransform;

static void Transform_free(void* ptr) {
    xfree(ptr);
}

static size_t Transform_size(const void* ptr) {
    (void)ptr;
    return sizeof(sfTransform);
}

static const rb_data_type_t Transform_data_type = {
    .wrap_struct_name = "SFML::Transform",
    .function = {.dmark = NULL, .dfree = Transform_free, .dsize = Transform_size},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

VALUE Get_Klass_Transform(void) {
    return rb_cTransform;
}

static sfTransform* Get_Transform_Struct(VALUE self) {
    sfTransform* ptr;
    TypedData_Get_Struct(self, sfTransform, &Transform_data_type, ptr);
    return ptr;
}

VALUE Transform_wrap(sfTransform c_transform) {
    sfTransform* transform = ALLOC(sfTransform);

    *transform = c_transform;

    return TypedData_Wrap_Struct(rb_cTransform, &Transform_data_type, transform);
}

/* Mutating in place on a caller's object, so the frozen check is not optional:
   IDENTITY is a frozen shared constant and must stay the identity. */
static sfTransform* Transform_writable(VALUE self) {
    rb_check_frozen(self);
    return Get_Transform_Struct(self);
}

/* -------------------------------------------------------------------------- */
/* Construction                                                               */
/* -------------------------------------------------------------------------- */

static VALUE Transform_alloc(VALUE klass) {
    sfTransform* transform = ALLOC(sfTransform);

    *transform = sfTransform_Identity;

    return TypedData_Wrap_Struct(klass, &Transform_data_type, transform);
}

/* call-seq:
 *   Transform.new                    -> identity Transform
 *   Transform.new(array_of_9)        -> Transform
 *   Transform.new(a00, a01, ... a22) -> Transform
 *
 * A Transform wraps a 3x3 matrix (9 floats, row-major). With no arguments it
 * is the identity transform; with a single 9-element Array or nine
 * individual numbers, it is built from those matrix elements directly.
 *
 * @return [Transform]
 * @raise [ArgumentError] if given an argument count other than 0, 1 or 9
 */
static VALUE Transform_init(int argc, VALUE* argv, VALUE self) {
    sfTransform* transform = Get_Transform_Struct(self);
    float matrix[MATRIX_LENGTH];

    if (argc == 0) {
        *transform = sfTransform_Identity;
        return self;
    }

    if (argc == 1) {
        Transform_ArrayToMatrix(argv[0], matrix);
    } else if (argc == MATRIX_LENGTH) {
        size_t i;

        for (i = 0; i < MATRIX_LENGTH; i++) {
            matrix[i] = (float)NUM2DBL(argv[i]);
        }
    } else {
        raise_invalid_arguments_excepted(MATRIX_LENGTH, (size_t)argc);
        return self;
    }

    *transform = sfTransform_fromMatrix(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4],
                                        matrix[5], matrix[6], matrix[7], matrix[8]);

    return self;
}

/* call-seq:
 *   Transform.identity -> Transform
 *
 * Returns the identity transform.
 *
 * @return [Transform] a fresh identity transform
 */
static VALUE Transform_s_identity(VALUE klass) {
    (void)klass;
    return Transform_wrap(sfTransform_Identity);
}

/* call-seq:
 *   Transform.from_a(array_of_9) -> Transform
 *
 * Creates a transform from a nine-element array.
 *
 * @return [Transform]
 * @raise [ArgumentError] if the Array has fewer than 9 elements
 */
static VALUE Transform_s_from_a(VALUE klass, VALUE rb_matrix) {
    (void)klass;
    return Transform_wrap(Transform_ArrayToTransform(rb_matrix));
}

/* -------------------------------------------------------------------------- */
/* Matrix access                                                              */
/* -------------------------------------------------------------------------- */

/* call-seq: to_a -> Array<Float>
 *
 * Returns the object as an Array.
 *
 * @return [Array<Float>] the 9 matrix elements, row-major
 */
static VALUE Transform_to_a(VALUE self) {
    return Transform_MatrixToArray(Get_Transform_Struct(self)->matrix);
}

/* The 16-float 4x4 that sfTransform_getMatrix() fills, ready for glLoadMatrixf.
   Deliberately not called #matrix: that name already means the 3x3 everywhere
   else in this binding (Sprite#matrix, Text#matrix, RenderState#matrix). */
/* call-seq: gl_matrix -> Array<Float>
 *
 * Returns the transform as a 4x4 OpenGL matrix.
 *
 * @return [Array<Float>] the equivalent 16-element 4x4 matrix, ready for
 *   +glLoadMatrixf+
 */
static VALUE Transform_gl_matrix(VALUE self) {
    float matrix[GL_MATRIX_LENGTH];
    VALUE rb_arr;
    size_t i;

    sfTransform_getMatrix(Get_Transform_Struct(self), matrix);

    rb_arr = rb_ary_new_capa(GL_MATRIX_LENGTH);

    for (i = 0; i < GL_MATRIX_LENGTH; i++) {
        rb_ary_push(rb_arr, DBL2NUM(matrix[i]));
    }

    return rb_arr;
}

/* call-seq:
 *   self == other -> true or false
 *
 * +other+ may be a Transform or a 9-element Array.
 *
 * @return [Boolean]
 */
static VALUE Transform_eql(VALUE self, VALUE rb_other) {
    sfTransform other;

    if (!rb_obj_is_kind_of(rb_other, rb_cTransform) && !RB_TYPE_P(rb_other, T_ARRAY)) {
        return Qfalse;
    }

    other = Transform_ArrayToTransform(rb_other);

    return BOOL2RB(sfTransform_equal(Get_Transform_Struct(self), &other));
}

/* call-seq: to_s -> String
 *
 * Returns a human-readable representation of the object.
 *
 * @return [String]
 */
static VALUE Transform_to_s(VALUE self) {
    const float* m = Get_Transform_Struct(self)->matrix;

    return rb_sprintf("#<SFML::Transform [%g, %g, %g] [%g, %g, %g] [%g, %g, %g]>", m[0], m[1], m[2],
                      m[3], m[4], m[5], m[6], m[7], m[8]);
}

/* -------------------------------------------------------------------------- */
/* Application                                                                */
/* -------------------------------------------------------------------------- */

/* call-seq:
 *   transform_point(point) -> Vector2
 *
 * Applies the transform to a point and returns the result.
 *
 * @return [Vector2] +point+ transformed by this matrix
 */
static VALUE Transform_transform_point(VALUE self, VALUE rb_point) {
    sfVector2f point =
        sfTransform_transformPoint(Get_Transform_Struct(self), vec2f_from_rb(rb_point));

    return vec2f_to_rb(point);
}

/* call-seq:
 *   transform_rect(rect) -> Rect
 *
 * Applies the transform to a rectangle and returns the result.
 *
 * @return [Rect] the axis-aligned bounding box of +rect+ transformed by
 *   this matrix
 */
static VALUE Transform_transform_rect(VALUE self, VALUE rb_rect) {
    sfFloatRect rect = sfTransform_transformRect(Get_Transform_Struct(self), rect_from_rb(rb_rect));

    return rect_to_rb(rect);
}

/* call-seq: inverse -> Transform
 *
 * Returns the inverse of the transform.
 *
 * @return [Transform] the inverse of this transform, or the identity if it
 *   is not invertible
 */
static VALUE Transform_get_inverse(VALUE self) {
    return Transform_wrap(sfTransform_getInverse(Get_Transform_Struct(self)));
}

/* call-seq: copy -> Transform
 *
 * Returns a deep copy of the object.
 *
 * @return [Transform] an independent copy
 */
static VALUE Transform_copy(VALUE self) {
    return Transform_wrap(*Get_Transform_Struct(self));
}

/* -------------------------------------------------------------------------- */
/* Mutators -- each returns self so they chain                                */
/* -------------------------------------------------------------------------- */

/* call-seq:
 *   combine!(other) -> self
 *
 * Combines this transform with +other+ (a Transform or 9-element Array) in
 * place, equivalent to matrix multiplication (+self+ * +other+).
 *
 * @return [self]
 */
static VALUE Transform_combine_bang(VALUE self, VALUE rb_other) {
    sfTransform other = Transform_ArrayToTransform(rb_other);

    sfTransform_combine(Transform_writable(self), &other);

    return self;
}

/* call-seq:
 *   self * other -> Transform
 *   combine(other) -> Transform
 *
 * Non-mutating combination: returns a new Transform equal to +self+
 * combined with +other+ (a Transform or 9-element Array), leaving both
 * operands untouched.
 *
 * @return [Transform]
 */
static VALUE Transform_mul(VALUE self, VALUE rb_other) {
    sfTransform combined = *Get_Transform_Struct(self);
    sfTransform other = Transform_ArrayToTransform(rb_other);

    sfTransform_combine(&combined, &other);

    return Transform_wrap(combined);
}

/* call-seq:
 *   translate!(offset) -> self
 *
 * Applies a translation to the transform in place.
 *
 * @return [self]
 */
static VALUE Transform_translate_bang(VALUE self, VALUE rb_offset) {
    sfTransform_translate(Transform_writable(self), vec2f_from_rb(rb_offset));
    return self;
}

/* call-seq:
 *   rotate!(angle)         -> self
 *   rotate!(angle, center) -> self
 *
 * Rotates by +angle+ degrees in place, around +center+ if given, otherwise
 * around the origin.
 *
 * @return [self]
 */
static VALUE Transform_rotate_bang(int argc, VALUE* argv, VALUE self) {
    VALUE rb_angle, rb_center;
    sfTransform* transform;

    rb_scan_args(argc, argv, "11", &rb_angle, &rb_center);
    transform = Transform_writable(self);

    if (NIL_P(rb_center)) {
        sfTransform_rotate(transform, (float)NUM2DBL(rb_angle));
    } else {
        sfTransform_rotateWithCenter(transform, (float)NUM2DBL(rb_angle), vec2f_from_rb(rb_center));
    }

    return self;
}

/* call-seq:
 *   scale!(factors)         -> self
 *   scale!(factors, center) -> self
 *
 * Scales by +factors+ (a Vector2 or 2-element Array) in place, around
 * +center+ if given, otherwise around the origin.
 *
 * @return [self]
 */
static VALUE Transform_scale_bang(int argc, VALUE* argv, VALUE self) {
    VALUE rb_factors, rb_center;
    sfTransform* transform;

    rb_scan_args(argc, argv, "11", &rb_factors, &rb_center);
    transform = Transform_writable(self);

    if (NIL_P(rb_center)) {
        sfTransform_scale(transform, vec2f_from_rb(rb_factors));
    } else {
        sfTransform_scaleWithCenter(transform, vec2f_from_rb(rb_factors), vec2f_from_rb(rb_center));
    }

    return self;
}

/* Non-mutating forms: copy, then apply. */
/* call-seq:
 *   translate(offset) -> Transform
 *
 * Returns a copy translated by the given offsets.
 *
 * @return [Transform] a new transform, translated by +offset+
 */
static VALUE Transform_translate(VALUE self, VALUE rb_offset) {
    return Transform_translate_bang(Transform_copy(self), rb_offset);
}

/* call-seq:
 *   rotate(angle)         -> Transform
 *   rotate(angle, center) -> Transform
 *
 * Returns a copy rotated by +angle+ degrees, around +center+ if given.
 *
 * @return [Transform] a new transform, rotated by +angle+ degrees around
 *   +center+ if given, otherwise around the origin
 */
static VALUE Transform_rotate(int argc, VALUE* argv, VALUE self) {
    return Transform_rotate_bang(argc, argv, Transform_copy(self));
}

/* call-seq:
 *   scale(factors)         -> Transform
 *   scale(factors, center) -> Transform
 *
 * Returns a copy scaled by +factors+, around +center+ if given.
 *
 * @return [Transform] a new transform, scaled by +factors+ around +center+
 *   if given, otherwise around the origin
 */
static VALUE Transform_scale(int argc, VALUE* argv, VALUE self) {
    return Transform_scale_bang(argc, argv, Transform_copy(self));
}

/* -------------------------------------------------------------------------- */
/* Deprecated module-function API                                             */
/* -------------------------------------------------------------------------- */

/* Transform used to be a module whose two functions took and returned plain
   9-element Arrays. Both are kept, Array-in/Array-out, so existing callers are
   unaffected by the promotion to a class. */
/* call-seq:
 *   Transform.combine(array_a, array_b) -> Array<Float>
 *
 * Returns the transform that combines +array_a+ and +array_b+.
 *
 * @deprecated Use #combine on a Transform instead.
 * @return [Array<Float>] the 9-element matrix of +array_a+ combined with
 *   +array_b+
 */
static VALUE Transform_s_combine(VALUE klass, VALUE rb_arr_a, VALUE rb_arr_b) {
    sfTransform transform_a = Transform_ArrayToTransform(rb_arr_a);
    sfTransform transform_b = Transform_ArrayToTransform(rb_arr_b);

    (void)klass;

    sfTransform_combine(&transform_a, &transform_b);

    return Transform_MatrixToArray(transform_a.matrix);
}

/* call-seq:
 *   Transform.inverse(array) -> Array<Float>
 *
 * Returns the inverse of the given transform.
 *
 * @deprecated Use #inverse on a Transform instead.
 * @return [Array<Float>] the 9-element inverse matrix of +array+
 */
static VALUE Transform_s_inverse(VALUE klass, VALUE rb_matrix) {
    sfTransform transform = Transform_ArrayToTransform(rb_matrix);

    (void)klass;

    return Transform_MatrixToArray(sfTransform_getInverse(&transform).matrix);
}

/* Document-class: SFML::Transform
 * A 3x3 matrix describing a 2D transformation (translation, rotation,
 * scaling, or any combination). Used to build up and query the
 * transformations applied by Sprite, Text and Transformable.
 *
 * @!attribute [r] matrix
 *   The object's 3x3 transform matrix.
 *   @return [Array<Float>] the 9 matrix elements, row-major (alias for #to_a)
 */
void Init_Transform(VALUE rb_mSFML) {
    rb_cTransform = rb_define_class_under(rb_mSFML, "Transform", rb_cObject);

    rb_define_alloc_func(rb_cTransform, Transform_alloc);
    rb_define_method(rb_cTransform, "initialize", Transform_init, -1);

    rb_define_singleton_method(rb_cTransform, "identity", Transform_s_identity, 0);
    rb_define_singleton_method(rb_cTransform, "from_a", Transform_s_from_a, 1);
    rb_define_singleton_method(rb_cTransform, "combine", Transform_s_combine, 2);
    rb_define_singleton_method(rb_cTransform, "inverse", Transform_s_inverse, 1);

    rb_define_method(rb_cTransform, "to_a", Transform_to_a, 0);
    rb_define_method(rb_cTransform, "to_ary", Transform_to_a, 0);
    rb_define_method(rb_cTransform, "matrix", Transform_to_a, 0);
    rb_define_method(rb_cTransform, "gl_matrix", Transform_gl_matrix, 0);
    rb_define_method(rb_cTransform, "==", Transform_eql, 1);
    rb_define_method(rb_cTransform, "to_s", Transform_to_s, 0);
    rb_define_method(rb_cTransform, "inspect", Transform_to_s, 0);

    rb_define_method(rb_cTransform, "copy", Transform_copy, 0);
    rb_define_method(rb_cTransform, "inverse", Transform_get_inverse, 0);
    rb_define_method(rb_cTransform, "transform_point", Transform_transform_point, 1);
    rb_define_method(rb_cTransform, "transform_rect", Transform_transform_rect, 1);

    rb_define_method(rb_cTransform, "*", Transform_mul, 1);
    rb_define_method(rb_cTransform, "combine", Transform_mul, 1);
    rb_define_method(rb_cTransform, "combine!", Transform_combine_bang, 1);
    rb_define_method(rb_cTransform, "translate", Transform_translate, 1);
    rb_define_method(rb_cTransform, "translate!", Transform_translate_bang, 1);
    rb_define_method(rb_cTransform, "rotate", Transform_rotate, -1);
    rb_define_method(rb_cTransform, "rotate!", Transform_rotate_bang, -1);
    rb_define_method(rb_cTransform, "scale", Transform_scale, -1);
    rb_define_method(rb_cTransform, "scale!", Transform_scale_bang, -1);

    /* The identity transform, frozen. */
    rb_define_const(rb_cTransform, "IDENTITY", rb_obj_freeze(Transform_wrap(sfTransform_Identity)));
}

/* -------------------------------------------------------------------------- */
/* Conversion helpers, shared with every class that exposes a transform        */
/* -------------------------------------------------------------------------- */

VALUE Transform_MatrixToArray(float* c_matrix) {
    VALUE rb_arr;
    size_t i;

    if (c_matrix == NULL) {
        return Qnil;
    }

    rb_arr = rb_ary_new_capa(MATRIX_LENGTH);

    for (i = 0; i < MATRIX_LENGTH; i++) {
        rb_ary_push(rb_arr, DBL2NUM(c_matrix[i]));
    }

    return rb_arr;
}

void Transform_ArrayToMatrix(VALUE rb_matrix, float* c_matrix) {
    size_t i;

    if (c_matrix == NULL) {
        return;
    }

    if (rb_obj_is_kind_of(rb_matrix, rb_cTransform)) {
        const float* source = Get_Transform_Struct(rb_matrix)->matrix;

        for (i = 0; i < MATRIX_LENGTH; i++) {
            c_matrix[i] = source[i];
        }

        return;
    }

    if (!RB_TYPE_P(rb_matrix, T_ARRAY)) {
        raise_invalid_argument_class(rb_cTransform);
    }

    if (rb_array_len(rb_matrix) != MATRIX_LENGTH) {
        raise_invalid_array_length(MATRIX_LENGTH);
    }

    for (i = 0; i < MATRIX_LENGTH; i++) {
        c_matrix[i] = (float)NUM2DBL(rb_ary_entry(rb_matrix, (long)i));
    }
}

sfTransform Transform_ArrayToTransform(VALUE rb_matrix) {
    sfTransform transform;

    Transform_ArrayToMatrix(rb_matrix, transform.matrix);

    return transform;
}

VALUE Transform_TransformToArray(sfTransform c_transform) {
    return Transform_MatrixToArray(c_transform.matrix);
}

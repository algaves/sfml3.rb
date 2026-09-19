#include "system/vec3.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfVector3f vec;
} Vector3;

static VALUE rb_cVector3;

static void Vector3_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Vector3_data_type = {
    .wrap_struct_name = "SFML::Vector3",
    .function = {.dmark = NULL, .dfree = Vector3_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

/* call-seq:
 *   Vector3.new                -> Vector3(0, 0, 0)
 *   Vector3.new(x, y, z)       -> Vector3(x, y, z)
 *   Vector3.new([x, y, z])     -> Vector3(x, y, z)
 *   Vector3.new(other_vector3) -> copy of +other_vector3+
 *
 * Creates a new 3D vector from three coordinates, a 3-element Array, or
 * another Vector3.
 *
 * @return [Vector3]
 * @raise [ArgumentError] if given an Array shorter than 3 elements, or an
 *   argument count other than 0, 1 or 3
 */
static VALUE Vector3_new(int argc, VALUE* argv, VALUE klass) {
    VALUE self;
    Vector3* ptr;
    float x = 0;
    float y = 0;
    float z = 0;

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cVector3)) {
        ptr = Get_Vector3_Struct(argv[0]);
        x = ptr->vec.x;
        y = ptr->vec.y;
        z = ptr->vec.z;
    } else if (argc == 1 && RB_TYPE_P(argv[0], T_ARRAY)) {
        if (RARRAY_LEN(argv[0]) < 3) {
            raise_invalid_array_length(3);
        }

        x = NUM2DBL(rb_ary_entry(argv[0], 0));
        y = NUM2DBL(rb_ary_entry(argv[0], 1));
        z = NUM2DBL(rb_ary_entry(argv[0], 2));
    } else if (argc == 3) {
        x = NUM2DBL(argv[0]);
        y = NUM2DBL(argv[1]);
        z = NUM2DBL(argv[2]);
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(3, argc);
    }

    ptr = malloc(sizeof(Vector3));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate vector");
    }

    ptr->vec.x = x;
    ptr->vec.y = y;
    ptr->vec.z = z;

    self = TypedData_Wrap_Struct(klass, &Vector3_data_type, ptr);

    return self;
}

/* call-seq: x -> Float
 *
 * Returns the X component of the vector.
 *
 * @return [Float] the X component
 */
static VALUE Vector3_get_x(VALUE self) {
    return DBL2NUM(((Vector3*)Get_Vector3_Struct(self))->vec.x);
}

/* call-seq: y -> Float
 *
 * Returns the Y component of the vector.
 *
 * @return [Float] the Y component
 */
static VALUE Vector3_get_y(VALUE self) {
    return DBL2NUM(((Vector3*)Get_Vector3_Struct(self))->vec.y);
}

/* call-seq: z -> Float
 *
 * Returns the Z component of the vector.
 *
 * @return [Float] the Z component
 */
static VALUE Vector3_get_z(VALUE self) {
    return DBL2NUM(((Vector3*)Get_Vector3_Struct(self))->vec.z);
}

/* call-seq:
 *   x=(value) -> Float
 *
 * Sets the X component. +value+ is any Float-convertible number.
 *
 * @return [Float] +value+
 */
static VALUE Vector3_set_x(VALUE self, VALUE rb_x) {
    ((Vector3*)Get_Vector3_Struct(self))->vec.x = NUM2DBL(rb_x);
    return rb_x;
}

/* call-seq:
 *   y=(value) -> Float
 *
 * Sets the Y component. +value+ is any Float-convertible number.
 *
 * @return [Float] +value+
 */
static VALUE Vector3_set_y(VALUE self, VALUE rb_y) {
    ((Vector3*)Get_Vector3_Struct(self))->vec.y = NUM2DBL(rb_y);
    return rb_y;
}

/* call-seq:
 *   z=(value) -> Float
 *
 * Sets the Z component. +value+ is any Float-convertible number.
 *
 * @return [Float] +value+
 */
static VALUE Vector3_set_z(VALUE self, VALUE rb_z) {
    ((Vector3*)Get_Vector3_Struct(self))->vec.z = NUM2DBL(rb_z);
    return rb_z;
}

/* call-seq: to_a -> Array
 *
 * Returns the vector's components as a three-element Array.
 *
 * @return [Array<Float>] +[x, y, z]+
 */
static VALUE Vector3_to_a(VALUE self) {
    Vector3* vec = Get_Vector3_Struct(self);

    return rb_ary_new_from_args(3, DBL2NUM(vec->vec.x), DBL2NUM(vec->vec.y), DBL2NUM(vec->vec.z));
}

/* call-seq: each { |component| ... } -> self
 *
 * Yields +x+, +y+, then +z+.
 *
 * @return [self]
 */
static VALUE Vector3_each(VALUE self) {
    Vector3* vec = Get_Vector3_Struct(self);

    rb_yield(DBL2NUM(vec->vec.x));
    rb_yield(DBL2NUM(vec->vec.y));
    rb_yield(DBL2NUM(vec->vec.z));

    return self;
}

/* call-seq: size -> Integer
 *
 * Returns the number of components, always 3.
 *
 * @return [Integer] always 3
 */
static VALUE Vector3_size(VALUE self) {
    return INT2NUM(3);
}

/* call-seq:
 *   self == other -> true or false
 *
 * +other+ may be a Vector3 or a 3-element Array.
 *
 * @return [Boolean]
 */
static VALUE Vector3_eql(VALUE self, VALUE rb_other) {
    Vector3* a = Get_Vector3_Struct(self);
    sfVector3f b;

    if (rb_obj_is_kind_of(rb_other, rb_cVector3)) {
        b = ((Vector3*)Get_Vector3_Struct(rb_other))->vec;
    } else if (RB_TYPE_P(rb_other, T_ARRAY) && RARRAY_LEN(rb_other) >= 3) {
        b = vec3f_from_rb(rb_other);
    } else {
        return Qfalse;
    }

    return BOOL2RB(a->vec.x == b.x && a->vec.y == b.y && a->vec.z == b.z);
}

/* call-seq:
 *   self + other -> Vector3
 *
 * +other+ may be a Vector3 or a 3-element Array.
 *
 * @return [Vector3] componentwise sum
 */
static VALUE Vector3_add(VALUE self, VALUE rb_other) {
    Vector3* a = Get_Vector3_Struct(self);
    sfVector3f b = vec3f_from_rb(rb_other);

    return vec3f_to_rb((sfVector3f){a->vec.x + b.x, a->vec.y + b.y, a->vec.z + b.z});
}

/* call-seq:
 *   self - other -> Vector3
 *
 * +other+ may be a Vector3 or a 3-element Array.
 *
 * @return [Vector3] componentwise difference
 */
static VALUE Vector3_sub(VALUE self, VALUE rb_other) {
    Vector3* a = Get_Vector3_Struct(self);
    sfVector3f b = vec3f_from_rb(rb_other);

    return vec3f_to_rb((sfVector3f){a->vec.x - b.x, a->vec.y - b.y, a->vec.z - b.z});
}

/* call-seq:
 *   self * scalar -> Vector3
 *
 * Multiplies all three components by +scalar+.
 *
 * @return [Vector3]
 */
static VALUE Vector3_mul(VALUE self, VALUE rb_scalar) {
    Vector3* a = Get_Vector3_Struct(self);
    float scalar = NUM2DBL(rb_scalar);

    return vec3f_to_rb((sfVector3f){a->vec.x * scalar, a->vec.y * scalar, a->vec.z * scalar});
}

/* call-seq: to_s -> String
 *
 * Returns a human-readable +"(x, y, z)"+ representation.
 *
 * @return [String] +"(x, y, z)"+
 */
static VALUE Vector3_to_s(VALUE self) {
    Vector3* a = Get_Vector3_Struct(self);
    char buffer[96];

    snprintf(buffer, sizeof(buffer), "(%g, %g, %g)", a->vec.x, a->vec.y, a->vec.z);

    return rb_str_new2(buffer);
}

/* Document-class: SFML::Vector3
 * A 3D vector of floats, used for positions, sizes and directions in 3D
 * space (e.g. Listener and SoundSource position/direction/velocity).
 *
 * Includes +Enumerable+ and behaves like a 3-element sequence: it responds to
 * #each and #to_a, and can be compared or combined with a plain
 * +[x, y, z]+ Array anywhere a Vector3 is accepted.
 *
 * @!attribute x
 *   Returns the X component.
 *
 *   @return [Float] the X component
 * @!attribute y
 *   Returns the Y component.
 *
 *   @return [Float] the Y component
 * @!attribute z
 *   Returns the Z component.
 *
 *   @return [Float] the Z component
 */
void Init_Vector3(VALUE rb_mSFML) {
    rb_cVector3 = rb_define_class_under(rb_mSFML, "Vector3", rb_cObject);

    rb_include_module(rb_cVector3, rb_mEnumerable);

    rb_define_singleton_method(rb_cVector3, "new", Vector3_new, -1);

    rb_define_method(rb_cVector3, "x", Vector3_get_x, 0);
    rb_define_method(rb_cVector3, "y", Vector3_get_y, 0);
    rb_define_method(rb_cVector3, "z", Vector3_get_z, 0);
    rb_define_method(rb_cVector3, "x=", Vector3_set_x, 1);
    rb_define_method(rb_cVector3, "y=", Vector3_set_y, 1);
    rb_define_method(rb_cVector3, "z=", Vector3_set_z, 1);

    rb_define_method(rb_cVector3, "to_a", Vector3_to_a, 0);
    rb_define_method(rb_cVector3, "to_ary", Vector3_to_a, 0);
    rb_define_method(rb_cVector3, "each", Vector3_each, 0);
    rb_define_method(rb_cVector3, "size", Vector3_size, 0);
    rb_define_method(rb_cVector3, "length", Vector3_size, 0);
    rb_define_method(rb_cVector3, "==", Vector3_eql, 1);
    rb_define_method(rb_cVector3, "+", Vector3_add, 1);
    rb_define_method(rb_cVector3, "-", Vector3_sub, 1);
    rb_define_method(rb_cVector3, "*", Vector3_mul, 1);
    rb_define_method(rb_cVector3, "to_s", Vector3_to_s, 0);
}

VALUE Get_Klass_Vector3(void) {
    return rb_cVector3;
}

void* Get_Vector3_Struct(VALUE self) {
    Vector3* ptr;
    TypedData_Get_Struct(self, Vector3, &Vector3_data_type, ptr);
    return ptr;
}

sfVector3f vec3f_from_rb(VALUE rb_vec) {
    if (rb_obj_is_kind_of(rb_vec, rb_cVector3)) {
        return ((Vector3*)Get_Vector3_Struct(rb_vec))->vec;
    }

    if (RB_TYPE_P(rb_vec, T_ARRAY)) {
        if (RARRAY_LEN(rb_vec) < 3) {
            raise_invalid_array_length(3);
        }

        return (sfVector3f){(float)NUM2DBL(rb_ary_entry(rb_vec, 0)),
                            (float)NUM2DBL(rb_ary_entry(rb_vec, 1)),
                            (float)NUM2DBL(rb_ary_entry(rb_vec, 2))};
    }

    raise_invalid_argument_class(rb_cVector3);

    return (sfVector3f){0, 0, 0};
}

VALUE vec3f_to_rb(sfVector3f c_vec) {
    Vector3* ptr = malloc(sizeof(Vector3));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate vector");
    }

    ptr->vec = c_vec;

    return TypedData_Wrap_Struct(rb_cVector3, &Vector3_data_type, ptr);
}

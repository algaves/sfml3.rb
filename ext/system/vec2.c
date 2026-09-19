#include "system/vec2.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfVector2f vec;
} Vector2;

static VALUE rb_cVector2;

static void Vector2_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Vector2_data_type = {
    .wrap_struct_name = "SFML::Vector2",
    .function = {.dmark = NULL, .dfree = Vector2_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

/* call-seq:
 *   Vector2.new                -> Vector2(0, 0)
 *   Vector2.new(x, y)          -> Vector2(x, y)
 *   Vector2.new([x, y])        -> Vector2(x, y)
 *   Vector2.new(other_vector2) -> copy of +other_vector2+
 *
 * Creates a new 2D vector from two coordinates, a 2-element Array, or
 * another Vector2.
 *
 * @return [Vector2]
 * @raise [ArgumentError] if given an Array shorter than 2 elements, or an
 *   argument count other than 0, 1 or 2
 */
static VALUE Vector2_new(int argc, VALUE* argv, VALUE klass) {
    VALUE self;
    Vector2* ptr;
    float x = 0;
    float y = 0;

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cVector2)) {
        ptr = Get_Vector2_Struct(argv[0]);
        x = ptr->vec.x;
        y = ptr->vec.y;
    } else if (argc == 1 && RB_TYPE_P(argv[0], T_ARRAY)) {
        if (RARRAY_LEN(argv[0]) < 2) {
            raise_invalid_array_length(2);
        }

        x = NUM2DBL(rb_ary_entry(argv[0], 0));
        y = NUM2DBL(rb_ary_entry(argv[0], 1));
    } else if (argc == 2) {
        x = NUM2DBL(argv[0]);
        y = NUM2DBL(argv[1]);
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(2, argc);
    }

    ptr = malloc(sizeof(Vector2));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate vector");
    }

    ptr->vec.x = x;
    ptr->vec.y = y;

    self = TypedData_Wrap_Struct(klass, &Vector2_data_type, ptr);

    return self;
}

/* call-seq: x -> Float
 *
 * Returns the X component of the vector.
 *
 * @return [Float] the X component
 */
static VALUE Vector2_get_x(VALUE self) {
    return DBL2NUM(((Vector2*)Get_Vector2_Struct(self))->vec.x);
}

/* call-seq: y -> Float
 *
 * Returns the Y component of the vector.
 *
 * @return [Float] the Y component
 */
static VALUE Vector2_get_y(VALUE self) {
    return DBL2NUM(((Vector2*)Get_Vector2_Struct(self))->vec.y);
}

/* call-seq:
 *   x=(value) -> Float
 *
 * Sets the X component. +value+ is any Float-convertible number.
 *
 * @return [Float] +value+
 */
static VALUE Vector2_set_x(VALUE self, VALUE rb_x) {
    ((Vector2*)Get_Vector2_Struct(self))->vec.x = NUM2DBL(rb_x);
    return rb_x;
}

/* call-seq:
 *   y=(value) -> Float
 *
 * Sets the Y component. +value+ is any Float-convertible number.
 *
 * @return [Float] +value+
 */
static VALUE Vector2_set_y(VALUE self, VALUE rb_y) {
    ((Vector2*)Get_Vector2_Struct(self))->vec.y = NUM2DBL(rb_y);
    return rb_y;
}

/* call-seq: to_a -> [Float, Float]
 *
 * Returns the vector's components as a two-element Array.
 *
 * @return [Array<Float>] +[x, y]+
 */
static VALUE Vector2_to_a(VALUE self) {
    Vector2* vec = Get_Vector2_Struct(self);

    return rb_ary_new_from_args(2, DBL2NUM(vec->vec.x), DBL2NUM(vec->vec.y));
}

/* call-seq: each { |component| ... } -> self
 *
 * Yields +x+ then +y+.
 *
 * @return [self]
 */
static VALUE Vector2_each(VALUE self) {
    Vector2* vec = Get_Vector2_Struct(self);

    rb_yield(DBL2NUM(vec->vec.x));
    rb_yield(DBL2NUM(vec->vec.y));

    return self;
}

/* call-seq:
 *   [](index) -> Float
 *
 * Returns the component stored at +index+.
 *
 * @return [Float] +x+ for index 0, +y+ for index 1
 */
static VALUE Vector2_aref(VALUE self, VALUE rb_index) {
    return rb_ary_entry(Vector2_to_a(self), NUM2LONG(rb_index));
}

/* call-seq: size -> Integer
 *
 * Returns the number of components, always 2.
 *
 * @return [Integer] always 2
 */
static VALUE Vector2_size(VALUE self) {
    return INT2NUM(2);
}

/* call-seq:
 *   self == other -> true or false
 *
 * +other+ may be a Vector2 or a 2-element Array.
 *
 * @return [Boolean]
 */
static VALUE Vector2_eql(VALUE self, VALUE rb_other) {
    Vector2* a = Get_Vector2_Struct(self);
    sfVector2f b;

    if (rb_obj_is_kind_of(rb_other, rb_cVector2)) {
        b = ((Vector2*)Get_Vector2_Struct(rb_other))->vec;
    } else if (RB_TYPE_P(rb_other, T_ARRAY) && RARRAY_LEN(rb_other) >= 2) {
        b = vec2f_from_rb(rb_other);
    } else {
        return Qfalse;
    }

    return BOOL2RB(a->vec.x == b.x && a->vec.y == b.y);
}

/* call-seq:
 *   self + other -> Vector2
 *
 * +other+ may be a Vector2 or a 2-element Array.
 *
 * @return [Vector2] componentwise sum
 */
static VALUE Vector2_add(VALUE self, VALUE rb_other) {
    Vector2* a = Get_Vector2_Struct(self);
    sfVector2f b = vec2f_from_rb(rb_other);

    return vec2f_to_rb((sfVector2f){a->vec.x + b.x, a->vec.y + b.y});
}

/* call-seq:
 *   self - other -> Vector2
 *
 * +other+ may be a Vector2 or a 2-element Array.
 *
 * @return [Vector2] componentwise difference
 */
static VALUE Vector2_sub(VALUE self, VALUE rb_other) {
    Vector2* a = Get_Vector2_Struct(self);
    sfVector2f b = vec2f_from_rb(rb_other);

    return vec2f_to_rb((sfVector2f){a->vec.x - b.x, a->vec.y - b.y});
}

/* call-seq:
 *   self * scalar -> Vector2
 *
 * Multiplies both components by +scalar+.
 *
 * @return [Vector2]
 */
static VALUE Vector2_mul(VALUE self, VALUE rb_scalar) {
    Vector2* a = Get_Vector2_Struct(self);
    float scalar = NUM2DBL(rb_scalar);

    return vec2f_to_rb((sfVector2f){a->vec.x * scalar, a->vec.y * scalar});
}

/* call-seq: to_s -> String
 *
 * Returns a human-readable +"(x, y)"+ representation.
 *
 * @return [String] +"(x, y)"+
 */
static VALUE Vector2_to_s(VALUE self) {
    Vector2* a = Get_Vector2_Struct(self);
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "(%g, %g)", a->vec.x, a->vec.y);

    return rb_str_new2(buffer);
}

/* Document-class: SFML::Vector2
 * A 2D vector of floats, used throughout the library for positions, sizes,
 * scale factors and directions.
 *
 * Includes +Enumerable+ and behaves like a 2-element sequence: it responds to
 * #each, #to_a and #[], and can be compared or combined with a plain
 * +[x, y]+ Array anywhere a Vector2 is accepted.
 *
 * @!attribute x
 *   Returns the X component.
 *
 *   @return [Float] the X component
 * @!attribute y
 *   Returns the Y component.
 *
 *   @return [Float] the Y component
 */
void Init_Vector2(VALUE rb_mSFML) {
    rb_cVector2 = rb_define_class_under(rb_mSFML, "Vector2", rb_cObject);

    rb_include_module(rb_cVector2, rb_mEnumerable);

    rb_define_singleton_method(rb_cVector2, "new", Vector2_new, -1);

    rb_define_method(rb_cVector2, "x", Vector2_get_x, 0);
    rb_define_method(rb_cVector2, "y", Vector2_get_y, 0);
    rb_define_method(rb_cVector2, "x=", Vector2_set_x, 1);
    rb_define_method(rb_cVector2, "y=", Vector2_set_y, 1);

    rb_define_method(rb_cVector2, "to_a", Vector2_to_a, 0);
    rb_define_method(rb_cVector2, "to_ary", Vector2_to_a, 0);
    rb_define_method(rb_cVector2, "each", Vector2_each, 0);
    rb_define_method(rb_cVector2, "[]", Vector2_aref, 1);
    rb_define_method(rb_cVector2, "size", Vector2_size, 0);
    rb_define_method(rb_cVector2, "length", Vector2_size, 0);
    rb_define_method(rb_cVector2, "==", Vector2_eql, 1);
    rb_define_method(rb_cVector2, "+", Vector2_add, 1);
    rb_define_method(rb_cVector2, "-", Vector2_sub, 1);
    rb_define_method(rb_cVector2, "*", Vector2_mul, 1);
    rb_define_method(rb_cVector2, "to_s", Vector2_to_s, 0);
}

VALUE Get_Klass_Vector2(void) {
    return rb_cVector2;
}

void* Get_Vector2_Struct(VALUE self) {
    Vector2* ptr;
    TypedData_Get_Struct(self, Vector2, &Vector2_data_type, ptr);
    return ptr;
}

sfVector2f vec2f_from_rb(VALUE rb_vec) {
    if (rb_obj_is_kind_of(rb_vec, rb_cVector2)) {
        return ((Vector2*)Get_Vector2_Struct(rb_vec))->vec;
    }

    if (RB_TYPE_P(rb_vec, T_ARRAY)) {
        if (RARRAY_LEN(rb_vec) < 2) {
            raise_invalid_array_length(2);
        }

        return (sfVector2f){(float)NUM2DBL(rb_ary_entry(rb_vec, 0)),
                            (float)NUM2DBL(rb_ary_entry(rb_vec, 1))};
    }

    raise_invalid_argument_class(rb_cVector2);

    return (sfVector2f){0, 0};
}

sfVector2i vec2i_from_rb(VALUE rb_vec) {
    sfVector2f vec = vec2f_from_rb(rb_vec);

    return (sfVector2i){(int)vec.x, (int)vec.y};
}

sfVector2u vec2u_from_rb(VALUE rb_vec) {
    sfVector2f vec = vec2f_from_rb(rb_vec);

    return (sfVector2u){(unsigned)vec.x, (unsigned)vec.y};
}

VALUE vec2f_to_rb(sfVector2f c_vec) {
    Vector2* ptr = malloc(sizeof(Vector2));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate vector");
    }

    ptr->vec = c_vec;

    return TypedData_Wrap_Struct(rb_cVector2, &Vector2_data_type, ptr);
}

VALUE vec2_new(float x, float y) {
    return vec2f_to_rb((sfVector2f){x, y});
}

void vec2_check(VALUE rb_arr) {
    if (rb_obj_is_kind_of(rb_arr, rb_cVector2)) {
        return;
    }

    if (RB_TYPE_P(rb_arr, T_ARRAY) && RARRAY_LEN(rb_arr) >= 2) {
        return;
    }

    raise_invalid_array_length(2);
}

VALUE vec2f_new_from_c(sfVector2f c_vec) {
    return vec2f_to_rb(c_vec);
}

sfVector2i vec2i_new_from_ruby(VALUE rb_arr) {
    return vec2i_from_rb(rb_arr);
}

sfVector2u vec2u_new_from_ruby(VALUE rb_arr) {
    return vec2u_from_rb(rb_arr);
}

sfVector2f vec2f_new_from_ruby(VALUE rb_arr) {
    return vec2f_from_rb(rb_arr);
}

#include "graphics/shader.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/color.h"
#include "graphics/texture.h"
#include "system/input_stream.h"
#include "system/vec2.h"
#include "system/vec3.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cShader;

static void Shader_free(void *ptr) {
    sfShader_destroy(ptr);
}

static const rb_data_type_t Shader_data_type = {
    .wrap_struct_name = "SFML::Shader",
    .function = {.dmark = NULL, .dfree = Shader_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE Shader_wrap(VALUE klass, sfShader *shader) {
    if (shader == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create shader");
    }

    return TypedData_Wrap_Struct(klass, &Shader_data_type, shader);
}

/* NULL skips a stage, mirroring CSFML. */
static const char *Shader_optional_cstr(VALUE rb_value) {
    if (NIL_P(rb_value)) {
        return NULL;
    }

    return StringValueCStr(rb_value);
}

static VALUE Shader_from_file(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_vertex, rb_fragment, rb_geometry;

    rb_scan_args(argc, argv, "12", &rb_vertex, &rb_fragment, &rb_geometry);

    return Shader_wrap(klass, sfShader_createFromFile(StringValueCStr(rb_vertex),
                                                      Shader_optional_cstr(rb_geometry),
                                                      Shader_optional_cstr(rb_fragment)));
}

static VALUE Shader_from_memory(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_vertex, rb_fragment, rb_geometry;

    rb_scan_args(argc, argv, "12", &rb_vertex, &rb_fragment, &rb_geometry);

    return Shader_wrap(klass, sfShader_createFromMemory(Shader_optional_cstr(rb_vertex),
                                                        Shader_optional_cstr(rb_geometry),
                                                        Shader_optional_cstr(rb_fragment)));
}

static VALUE Shader_from_stream(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_vertex, rb_fragment, rb_geometry;
    VALUE holder_vertex = Qnil, holder_fragment = Qnil, holder_geometry = Qnil;
    sfInputStream *vertex = NULL;
    sfInputStream *fragment = NULL;
    sfInputStream *geometry = NULL;

    rb_scan_args(argc, argv, "12", &rb_vertex, &rb_fragment, &rb_geometry);

    if (!NIL_P(rb_vertex)) {
        vertex = input_stream_from_rb(rb_vertex, &holder_vertex);
    }

    if (!NIL_P(rb_fragment)) {
        fragment = input_stream_from_rb(rb_fragment, &holder_fragment);
    }

    if (!NIL_P(rb_geometry)) {
        geometry = input_stream_from_rb(rb_geometry, &holder_geometry);
    }

    (void) holder_vertex;
    (void) holder_fragment;
    (void) holder_geometry;

    return Shader_wrap(klass, sfShader_createFromStream(vertex, geometry, fragment));
}

static VALUE Shader_get_native_handle(VALUE self) {
    return UINT2NUM(sfShader_getNativeHandle(Get_Shader_Struct(self)));
}

static VALUE Shader_bind(VALUE self) {
    sfShader_bind(Get_Shader_Struct(self));
    return self;
}

static VALUE Shader_is_available(VALUE klass) {
    return BOOL2RB(sfShader_isAvailable());
}

static VALUE Shader_is_geometry_available(VALUE klass) {
    return BOOL2RB(sfShader_isGeometryAvailable());
}

static void Shader_name(VALUE rb_name, char *buffer, size_t length) {
    const char *name = StringValueCStr(rb_name);

    snprintf(buffer, length, "%s", name);
}

static VALUE Shader_set_float(VALUE self, VALUE rb_name, VALUE rb_x) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setFloatUniform((sfShader *) Get_Shader_Struct(self), name, NUM2DBL(rb_x));

    return self;
}

static VALUE Shader_set_int(VALUE self, VALUE rb_name, VALUE rb_x) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setIntUniform((sfShader *) Get_Shader_Struct(self), name, NUM2INT(rb_x));

    return self;
}

static VALUE Shader_set_bool(VALUE self, VALUE rb_name, VALUE rb_x) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setBoolUniform((sfShader *) Get_Shader_Struct(self), name, RTEST(rb_x));

    return self;
}

static VALUE Shader_set_color(VALUE self, VALUE rb_name, VALUE rb_color) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setColorUniform((sfShader *) Get_Shader_Struct(self), name, color_from_rb(rb_color));

    return self;
}

static VALUE Shader_set_int_color(VALUE self, VALUE rb_name, VALUE rb_color) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setIntColorUniform((sfShader *) Get_Shader_Struct(self), name, color_from_rb(rb_color));

    return self;
}

static VALUE Shader_set_vec2(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setVec2Uniform((sfShader *) Get_Shader_Struct(self), name, vec2f_from_rb(rb_vector));

    return self;
}

static VALUE Shader_set_vec3(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setVec3Uniform((sfShader *) Get_Shader_Struct(self), name, vec3f_from_rb(rb_vector));

    return self;
}

static VALUE Shader_set_vec4(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");
    sfGlslVec4 vector;

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 4) {
        raise_invalid_array_length(4);
    }

    vector.x = NUM2DBL(rb_ary_entry(array, 0));
    vector.y = NUM2DBL(rb_ary_entry(array, 1));
    vector.z = NUM2DBL(rb_ary_entry(array, 2));
    vector.w = NUM2DBL(rb_ary_entry(array, 3));

    sfShader_setVec4Uniform((sfShader *) Get_Shader_Struct(self), name, vector);

    return self;
}

static VALUE Shader_set_ivec2(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    sfVector2f vec = vec2f_from_rb(rb_vector);

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setIvec2Uniform((sfShader *) Get_Shader_Struct(self), name, (sfGlslIvec2) {(int) vec.x, (int) vec.y});

    return self;
}

static VALUE Shader_set_ivec3(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");
    sfGlslIvec3 vector;

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 3) {
        raise_invalid_array_length(3);
    }

    vector.x = NUM2INT(rb_ary_entry(array, 0));
    vector.y = NUM2INT(rb_ary_entry(array, 1));
    vector.z = NUM2INT(rb_ary_entry(array, 2));

    sfShader_setIvec3Uniform((sfShader *) Get_Shader_Struct(self), name, vector);

    return self;
}

static VALUE Shader_set_ivec4(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");
    sfGlslIvec4 vector;

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 4) {
        raise_invalid_array_length(4);
    }

    vector.x = NUM2INT(rb_ary_entry(array, 0));
    vector.y = NUM2INT(rb_ary_entry(array, 1));
    vector.z = NUM2INT(rb_ary_entry(array, 2));
    vector.w = NUM2INT(rb_ary_entry(array, 3));

    sfShader_setIvec4Uniform((sfShader *) Get_Shader_Struct(self), name, vector);

    return self;
}

static VALUE Shader_set_bvec2(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 2) {
        raise_invalid_array_length(2);
    }

    sfShader_setBvec2Uniform((sfShader *) Get_Shader_Struct(self), name,
                             (sfGlslBvec2) {RTEST(rb_ary_entry(array, 0)), RTEST(rb_ary_entry(array, 1))});

    return self;
}

static VALUE Shader_set_bvec3(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 3) {
        raise_invalid_array_length(3);
    }

    sfShader_setBvec3Uniform((sfShader *) Get_Shader_Struct(self), name,
                             (sfGlslBvec3) {RTEST(rb_ary_entry(array, 0)), RTEST(rb_ary_entry(array, 1)),
                                            RTEST(rb_ary_entry(array, 2))});

    return self;
}

static VALUE Shader_set_bvec4(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 4) {
        raise_invalid_array_length(4);
    }

    sfShader_setBvec4Uniform((sfShader *) Get_Shader_Struct(self), name,
                             (sfGlslBvec4) {RTEST(rb_ary_entry(array, 0)), RTEST(rb_ary_entry(array, 1)),
                                            RTEST(rb_ary_entry(array, 2)), RTEST(rb_ary_entry(array, 3))});

    return self;
}

static VALUE Shader_set_mat3(VALUE self, VALUE rb_name, VALUE rb_matrix) {
    char name[256];
    VALUE array = rb_convert_type(rb_matrix, T_ARRAY, "Array", "to_ary");
    sfGlslMat3 matrix;

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 9) {
        raise_invalid_array_length(9);
    }

    for (long i = 0; i < 9; i++) {
        matrix.array[i] = NUM2DBL(rb_ary_entry(array, i));
    }

    sfShader_setMat3Uniform((sfShader *) Get_Shader_Struct(self), name, &matrix);

    return self;
}

static VALUE Shader_set_mat4(VALUE self, VALUE rb_name, VALUE rb_matrix) {
    char name[256];
    VALUE array = rb_convert_type(rb_matrix, T_ARRAY, "Array", "to_ary");
    sfGlslMat4 matrix;

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 16) {
        raise_invalid_array_length(16);
    }

    for (long i = 0; i < 16; i++) {
        matrix.array[i] = NUM2DBL(rb_ary_entry(array, i));
    }

    sfShader_setMat4Uniform((sfShader *) Get_Shader_Struct(self), name, &matrix);

    return self;
}

static VALUE Shader_set_float_array(VALUE self, VALUE rb_name, VALUE rb_values) {
    char name[256];
    VALUE array = rb_convert_type(rb_values, T_ARRAY, "Array", "to_ary");
    long count = RARRAY_LEN(array);
    float *values = ALLOC_N(float, count);

    Shader_name(rb_name, name, sizeof(name));

    for (long i = 0; i < count; i++) {
        values[i] = NUM2DBL(rb_ary_entry(array, i));
    }

    sfShader_setFloatUniformArray((sfShader *) Get_Shader_Struct(self), name, values, (size_t) count);

    xfree(values);

    return self;
}

static VALUE Shader_set_vec2_array(VALUE self, VALUE rb_name, VALUE rb_values) {
    char name[256];
    VALUE array = rb_convert_type(rb_values, T_ARRAY, "Array", "to_ary");
    long count = RARRAY_LEN(array);
    sfGlslVec2 *values = ALLOC_N(sfGlslVec2, count);

    Shader_name(rb_name, name, sizeof(name));

    for (long i = 0; i < count; i++) {
        values[i] = vec2f_from_rb(rb_ary_entry(array, i));
    }

    sfShader_setVec2UniformArray((sfShader *) Get_Shader_Struct(self), name, values, (size_t) count);

    xfree(values);

    return self;
}

static VALUE Shader_set_vec3_array(VALUE self, VALUE rb_name, VALUE rb_values) {
    char name[256];
    VALUE array = rb_convert_type(rb_values, T_ARRAY, "Array", "to_ary");
    long count = RARRAY_LEN(array);
    sfGlslVec3 *values = ALLOC_N(sfGlslVec3, count);

    Shader_name(rb_name, name, sizeof(name));

    for (long i = 0; i < count; i++) {
        values[i] = vec3f_from_rb(rb_ary_entry(array, i));
    }

    sfShader_setVec3UniformArray((sfShader *) Get_Shader_Struct(self), name, values, (size_t) count);

    xfree(values);

    return self;
}

static VALUE Shader_set_texture(VALUE self, VALUE rb_name, VALUE rb_texture) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));

    if (NIL_P(rb_texture)) {
        sfShader_setTextureUniform((sfShader *) Get_Shader_Struct(self), name, NULL);
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    sfShader_setTextureUniform((sfShader *) Get_Shader_Struct(self), name, Get_Texture_Struct(rb_texture));

    return rb_texture;
}

static VALUE Shader_set_current_texture(VALUE self, VALUE rb_name) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setCurrentTextureUniform((sfShader *) Get_Shader_Struct(self), name);

    return self;
}

static VALUE Shader_set_uniform(VALUE self, VALUE rb_name, VALUE rb_value) {
    if (rb_obj_is_kind_of(rb_value, Get_Klass_Color())) {
        return Shader_set_color(self, rb_name, rb_value);
    }

    if (rb_obj_is_kind_of(rb_value, Get_Klass_Texture())) {
        return Shader_set_texture(self, rb_name, rb_value);
    }

    if (rb_obj_is_kind_of(rb_value, Get_Klass_Vector2())) {
        return Shader_set_vec2(self, rb_name, rb_value);
    }

    if (rb_obj_is_kind_of(rb_value, Get_Klass_Vector3())) {
        return Shader_set_vec3(self, rb_name, rb_value);
    }

    if (rb_value == Qtrue || rb_value == Qfalse) {
        return Shader_set_bool(self, rb_name, rb_value);
    }

    if (RB_INTEGER_TYPE_P(rb_value)) {
        return Shader_set_int(self, rb_name, rb_value);
    }

    if (RB_FLOAT_TYPE_P(rb_value)) {
        return Shader_set_float(self, rb_name, rb_value);
    }

    if (RB_TYPE_P(rb_value, T_ARRAY)) {
        long length = RARRAY_LEN(rb_value);

        if (length == 2) {
            return Shader_set_vec2(self, rb_name, rb_value);
        }

        if (length == 3) {
            return Shader_set_vec3(self, rb_name, rb_value);
        }

        if (length == 4) {
            return Shader_set_vec4(self, rb_name, rb_value);
        }

        if (length == 9) {
            return Shader_set_mat3(self, rb_name, rb_value);
        }

        if (length == 16) {
            return Shader_set_mat4(self, rb_name, rb_value);
        }
    }

    rb_raise(rb_eArgError, "unsupported shader uniform value");
    return self;
}

void Init_Shader(VALUE rb_module) {
    rb_cShader = rb_define_class_under(rb_module, "Shader", rb_cObject);

    rb_define_singleton_method(rb_cShader, "from_file", Shader_from_file, -1);
    rb_define_singleton_method(rb_cShader, "from_memory", Shader_from_memory, -1);
    rb_define_singleton_method(rb_cShader, "from_stream", Shader_from_stream, -1);
    rb_define_singleton_method(rb_cShader, "available?", Shader_is_available, 0);
    rb_define_singleton_method(rb_cShader, "geometry_available?", Shader_is_geometry_available, 0);

    rb_define_method(rb_cShader, "native_handle", Shader_get_native_handle, 0);
    rb_define_method(rb_cShader, "bind", Shader_bind, 0);

    rb_define_method(rb_cShader, "set_float", Shader_set_float, 2);
    rb_define_method(rb_cShader, "set_int", Shader_set_int, 2);
    rb_define_method(rb_cShader, "set_bool", Shader_set_bool, 2);
    rb_define_method(rb_cShader, "set_color", Shader_set_color, 2);
    rb_define_method(rb_cShader, "set_int_color", Shader_set_int_color, 2);
    rb_define_method(rb_cShader, "set_vec2", Shader_set_vec2, 2);
    rb_define_method(rb_cShader, "set_vec3", Shader_set_vec3, 2);
    rb_define_method(rb_cShader, "set_vec4", Shader_set_vec4, 2);
    rb_define_method(rb_cShader, "set_ivec2", Shader_set_ivec2, 2);
    rb_define_method(rb_cShader, "set_ivec3", Shader_set_ivec3, 2);
    rb_define_method(rb_cShader, "set_ivec4", Shader_set_ivec4, 2);
    rb_define_method(rb_cShader, "set_bvec2", Shader_set_bvec2, 2);
    rb_define_method(rb_cShader, "set_bvec3", Shader_set_bvec3, 2);
    rb_define_method(rb_cShader, "set_bvec4", Shader_set_bvec4, 2);
    rb_define_method(rb_cShader, "set_mat3", Shader_set_mat3, 2);
    rb_define_method(rb_cShader, "set_mat4", Shader_set_mat4, 2);
    rb_define_method(rb_cShader, "set_float_array", Shader_set_float_array, 2);
    rb_define_method(rb_cShader, "set_vec2_array", Shader_set_vec2_array, 2);
    rb_define_method(rb_cShader, "set_vec3_array", Shader_set_vec3_array, 2);
    rb_define_method(rb_cShader, "set_texture", Shader_set_texture, 2);
    rb_define_method(rb_cShader, "set_current_texture", Shader_set_current_texture, 1);
    rb_define_method(rb_cShader, "set_uniform", Shader_set_uniform, 2);
    rb_define_method(rb_cShader, "uniform=", Shader_set_uniform, 2);
}

VALUE Get_Klass_Shader(void) {
    return rb_cShader;
}

const sfShader *Get_Shader_Struct(VALUE self) {
    sfShader *ptr;
    TypedData_Get_Struct(self, sfShader, &Shader_data_type, ptr);
    return ptr;
}

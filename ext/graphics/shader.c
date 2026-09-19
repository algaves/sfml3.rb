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

/* SFML's Shader::setUniform(name, const Texture&) stores a pointer to the
   texture internally and re-binds it at draw/bind time, so (per SFML's own
   docs) the caller must keep every Texture passed to #set_texture alive for
   as long as the shader might use it -- hence the retaining Hash and the
   dmark this needs, unlike a bare pointer wrap. */
typedef struct {
    sfShader* shader;
    VALUE rb_textures;
} Shader;

static void Shader_mark(void* ptr) {
    rb_gc_mark(((Shader*)ptr)->rb_textures);
}

static void Shader_free(void* ptr) {
    Shader* wrapper = ptr;

    sfShader_destroy(wrapper->shader);
    free(wrapper);
}

static const rb_data_type_t Shader_data_type = {
    .wrap_struct_name = "SFML::Shader",
    .function = {.dmark = Shader_mark, .dfree = Shader_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Shader_wrap(VALUE klass, sfShader* c_shader) {
    Shader* wrapper;

    if (c_shader == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create shader");
    }

    wrapper = malloc(sizeof(Shader));

    if (wrapper == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate shader");
    }

    wrapper->shader = c_shader;
    wrapper->rb_textures = rb_hash_new();

    return TypedData_Wrap_Struct(klass, &Shader_data_type, wrapper);
}

/* NULL skips a stage, mirroring CSFML. */
static const char* Shader_optional_cstr(VALUE rb_value) {
    if (NIL_P(rb_value)) {
        return NULL;
    }

    return StringValueCStr(rb_value);
}

/* call-seq:
 *   Shader.from_file(vertex)                     -> Shader
 *   Shader.from_file(vertex, fragment)            -> Shader
 *   Shader.from_file(vertex, fragment, geometry)  -> Shader
 *
 * Compiles a shader from GLSL source file paths. Any of the three may be
 * +nil+ to skip that stage.
 *
 * @return [Shader]
 * @raise [RuntimeError] if compilation fails
 */
static VALUE Shader_from_file(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_vertex, rb_fragment, rb_geometry;

    rb_scan_args(argc, argv, "12", &rb_vertex, &rb_fragment, &rb_geometry);

    return Shader_wrap(klass, sfShader_createFromFile(StringValueCStr(rb_vertex),
                                                      Shader_optional_cstr(rb_geometry),
                                                      Shader_optional_cstr(rb_fragment)));
}

/* call-seq:
 *   Shader.from_memory(vertex)                    -> Shader
 *   Shader.from_memory(vertex, fragment)          -> Shader
 *   Shader.from_memory(vertex, fragment, geometry) -> Shader
 *
 * Compiles a shader from GLSL source Strings. Any of the three may be
 * +nil+ to skip that stage.
 *
 * @return [Shader]
 * @raise [RuntimeError] if compilation fails
 */
static VALUE Shader_from_memory(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_vertex, rb_fragment, rb_geometry;

    rb_scan_args(argc, argv, "12", &rb_vertex, &rb_fragment, &rb_geometry);

    return Shader_wrap(klass, sfShader_createFromMemory(Shader_optional_cstr(rb_vertex),
                                                        Shader_optional_cstr(rb_geometry),
                                                        Shader_optional_cstr(rb_fragment)));
}

/* call-seq:
 *   Shader.from_stream(vertex)                    -> Shader
 *   Shader.from_stream(vertex, fragment)          -> Shader
 *   Shader.from_stream(vertex, fragment, geometry) -> Shader
 *
 * Compiles a shader from GLSL source read through InputStream objects. Any
 * of the three may be +nil+ to skip that stage.
 *
 * @return [Shader]
 * @raise [RuntimeError] if compilation fails
 */
static VALUE Shader_from_stream(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_vertex, rb_fragment, rb_geometry;
    VALUE holder_vertex = Qnil, holder_fragment = Qnil, holder_geometry = Qnil;
    sfInputStream* vertex = NULL;
    sfInputStream* fragment = NULL;
    sfInputStream* geometry = NULL;

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

    (void)holder_vertex;
    (void)holder_fragment;
    (void)holder_geometry;

    return Shader_wrap(klass, sfShader_createFromStream(vertex, geometry, fragment));
}

/* call-seq: native_handle -> Integer
 *
 * @return [Integer] the underlying OpenGL program handle
 */
static VALUE Shader_get_native_handle(VALUE self) {
    return UINT2NUM(sfShader_getNativeHandle(Get_Shader_Struct(self)));
}

/* call-seq: bind -> self
 *
 * Activates this shader for rendering. Pass a RenderState carrying it to
 * Target#draw instead of calling this directly, unless drawing raw OpenGL.
 *
 * @return [self]
 */
static VALUE Shader_bind(VALUE self) {
    sfShader_bind(Get_Shader_Struct(self));
    return self;
}

/* call-seq:
 *   Shader.available? -> true or false
 *
 * @return [Boolean] whether the system supports shaders at all
 */
static VALUE Shader_is_available(VALUE klass) {
    return BOOL2RB(sfShader_isAvailable());
}

/* call-seq:
 *   Shader.geometry_available? -> true or false
 *
 * @return [Boolean] whether the system supports geometry shaders
 */
static VALUE Shader_is_geometry_available(VALUE klass) {
    return BOOL2RB(sfShader_isGeometryAvailable());
}

static void Shader_name(VALUE rb_name, char* buffer, size_t length) {
    const char* name = StringValueCStr(rb_name);

    snprintf(buffer, length, "%s", name);
}

/* call-seq:
 *   set_float(name, x) -> self
 *
 * @return [self]
 */
static VALUE Shader_set_float(VALUE self, VALUE rb_name, VALUE rb_x) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setFloatUniform((sfShader*)Get_Shader_Struct(self), name, NUM2DBL(rb_x));

    return self;
}

/* call-seq:
 *   set_int(name, x) -> self
 *
 * @return [self]
 */
static VALUE Shader_set_int(VALUE self, VALUE rb_name, VALUE rb_x) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setIntUniform((sfShader*)Get_Shader_Struct(self), name, NUM2INT(rb_x));

    return self;
}

/* call-seq:
 *   set_bool(name, x) -> self
 *
 * @return [self]
 */
static VALUE Shader_set_bool(VALUE self, VALUE rb_name, VALUE rb_x) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setBoolUniform((sfShader*)Get_Shader_Struct(self), name, RTEST(rb_x));

    return self;
}

/* call-seq:
 *   set_color(name, color) -> self
 *
 * Sets a +vec4+ uniform from +color+ (a Color), normalized to the 0..1
 * range.
 *
 * @return [self]
 */
static VALUE Shader_set_color(VALUE self, VALUE rb_name, VALUE rb_color) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setColorUniform((sfShader*)Get_Shader_Struct(self), name, color_from_rb(rb_color));

    return self;
}

/* call-seq:
 *   set_int_color(name, color) -> self
 *
 * Sets an +ivec4+ uniform from +color+ (a Color), in the 0..255 range.
 *
 * @return [self]
 */
static VALUE Shader_set_int_color(VALUE self, VALUE rb_name, VALUE rb_color) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setIntColorUniform((sfShader*)Get_Shader_Struct(self), name, color_from_rb(rb_color));

    return self;
}

/* call-seq:
 *   set_vec2(name, vector) -> self
 *
 * @return [self]
 */
static VALUE Shader_set_vec2(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setVec2Uniform((sfShader*)Get_Shader_Struct(self), name, vec2f_from_rb(rb_vector));

    return self;
}

/* call-seq:
 *   set_vec3(name, vector) -> self
 *
 * @return [self]
 */
static VALUE Shader_set_vec3(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setVec3Uniform((sfShader*)Get_Shader_Struct(self), name, vec3f_from_rb(rb_vector));

    return self;
}

/* Element converters shared by the scalar setters below and by the *_array
   setters, which need the same conversion once per element. */
static sfGlslVec4 glsl_vec4_from_rb(VALUE rb_vector) {
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");
    sfGlslVec4 vector;

    if (RARRAY_LEN(array) < 4) {
        raise_invalid_array_length(4);
    }

    vector.x = NUM2DBL(rb_ary_entry(array, 0));
    vector.y = NUM2DBL(rb_ary_entry(array, 1));
    vector.z = NUM2DBL(rb_ary_entry(array, 2));
    vector.w = NUM2DBL(rb_ary_entry(array, 3));

    return vector;
}

static sfGlslMat3 glsl_mat3_from_rb(VALUE rb_matrix) {
    VALUE array = rb_convert_type(rb_matrix, T_ARRAY, "Array", "to_ary");
    sfGlslMat3 matrix;
    long i;

    if (RARRAY_LEN(array) < 9) {
        raise_invalid_array_length(9);
    }

    for (i = 0; i < 9; i++) {
        matrix.array[i] = NUM2DBL(rb_ary_entry(array, i));
    }

    return matrix;
}

static sfGlslMat4 glsl_mat4_from_rb(VALUE rb_matrix) {
    VALUE array = rb_convert_type(rb_matrix, T_ARRAY, "Array", "to_ary");
    sfGlslMat4 matrix;
    long i;

    if (RARRAY_LEN(array) < 16) {
        raise_invalid_array_length(16);
    }

    for (i = 0; i < 16; i++) {
        matrix.array[i] = NUM2DBL(rb_ary_entry(array, i));
    }

    return matrix;
}

/* call-seq:
 *   set_vec4(name, vector) -> self
 *
 * +vector+ is a 4-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +vector+ has fewer than 4 elements
 */
static VALUE Shader_set_vec4(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    sfGlslVec4 vector = glsl_vec4_from_rb(rb_vector);

    Shader_name(rb_name, name, sizeof(name));

    sfShader_setVec4Uniform((sfShader*)Get_Shader_Struct(self), name, vector);

    return self;
}

/* call-seq:
 *   set_ivec2(name, vector) -> self
 *
 * @return [self]
 */
static VALUE Shader_set_ivec2(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    sfVector2f vec = vec2f_from_rb(rb_vector);

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setIvec2Uniform((sfShader*)Get_Shader_Struct(self), name,
                             (sfGlslIvec2){(int)vec.x, (int)vec.y});

    return self;
}

/* call-seq:
 *   set_ivec3(name, vector) -> self
 *
 * +vector+ is a 3-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +vector+ has fewer than 3 elements
 */
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

    sfShader_setIvec3Uniform((sfShader*)Get_Shader_Struct(self), name, vector);

    return self;
}

/* call-seq:
 *   set_ivec4(name, vector) -> self
 *
 * +vector+ is a 4-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +vector+ has fewer than 4 elements
 */
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

    sfShader_setIvec4Uniform((sfShader*)Get_Shader_Struct(self), name, vector);

    return self;
}

/* call-seq:
 *   set_bvec2(name, vector) -> self
 *
 * +vector+ is a 2-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +vector+ has fewer than 2 elements
 */
static VALUE Shader_set_bvec2(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 2) {
        raise_invalid_array_length(2);
    }

    sfShader_setBvec2Uniform(
        (sfShader*)Get_Shader_Struct(self), name,
        (sfGlslBvec2){RTEST(rb_ary_entry(array, 0)), RTEST(rb_ary_entry(array, 1))});

    return self;
}

/* call-seq:
 *   set_bvec3(name, vector) -> self
 *
 * +vector+ is a 3-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +vector+ has fewer than 3 elements
 */
static VALUE Shader_set_bvec3(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 3) {
        raise_invalid_array_length(3);
    }

    sfShader_setBvec3Uniform((sfShader*)Get_Shader_Struct(self), name,
                             (sfGlslBvec3){RTEST(rb_ary_entry(array, 0)),
                                           RTEST(rb_ary_entry(array, 1)),
                                           RTEST(rb_ary_entry(array, 2))});

    return self;
}

/* call-seq:
 *   set_bvec4(name, vector) -> self
 *
 * +vector+ is a 4-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +vector+ has fewer than 4 elements
 */
static VALUE Shader_set_bvec4(VALUE self, VALUE rb_name, VALUE rb_vector) {
    char name[256];
    VALUE array = rb_convert_type(rb_vector, T_ARRAY, "Array", "to_ary");

    Shader_name(rb_name, name, sizeof(name));

    if (RARRAY_LEN(array) < 4) {
        raise_invalid_array_length(4);
    }

    sfShader_setBvec4Uniform(
        (sfShader*)Get_Shader_Struct(self), name,
        (sfGlslBvec4){RTEST(rb_ary_entry(array, 0)), RTEST(rb_ary_entry(array, 1)),
                      RTEST(rb_ary_entry(array, 2)), RTEST(rb_ary_entry(array, 3))});

    return self;
}

/* call-seq:
 *   set_mat3(name, matrix) -> self
 *
 * +matrix+ is a 9-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +matrix+ has fewer than 9 elements
 */
static VALUE Shader_set_mat3(VALUE self, VALUE rb_name, VALUE rb_matrix) {
    char name[256];
    sfGlslMat3 matrix = glsl_mat3_from_rb(rb_matrix);

    Shader_name(rb_name, name, sizeof(name));

    sfShader_setMat3Uniform((sfShader*)Get_Shader_Struct(self), name, &matrix);

    return self;
}

/* call-seq:
 *   set_mat4(name, matrix) -> self
 *
 * +matrix+ is a 16-element Array.
 *
 * @return [self]
 * @raise [ArgumentError] if +matrix+ has fewer than 16 elements
 */
static VALUE Shader_set_mat4(VALUE self, VALUE rb_name, VALUE rb_matrix) {
    char name[256];
    sfGlslMat4 matrix = glsl_mat4_from_rb(rb_matrix);

    Shader_name(rb_name, name, sizeof(name));

    sfShader_setMat4Uniform((sfShader*)Get_Shader_Struct(self), name, &matrix);

    return self;
}

/* The six array setters differ only in element type, converter and CSFML
   entry point, so they are generated rather than written out six times.

   ALLOCV_N, not ALLOC_N: the converter runs per element and can raise (a bad
   element, a short vector), and ALLOCV's buffer is owned by a temporary VALUE
   that Ruby frees while unwinding. The hand-written versions this replaced
   leaked the buffer on that path. */
#define SHADER_ARRAY_UNIFORM(suffix, c_type, converter, sf_setter)                                 \
    static VALUE Shader_set_##suffix##_array(VALUE self, VALUE rb_name, VALUE rb_values) {         \
        char name[256];                                                                            \
        VALUE array = rb_convert_type(rb_values, T_ARRAY, "Array", "to_ary");                      \
        VALUE buffer;                                                                              \
        long count = RARRAY_LEN(array);                                                            \
        long i;                                                                                    \
        c_type* values = ALLOCV_N(c_type, buffer, count);                                          \
                                                                                                   \
        Shader_name(rb_name, name, sizeof(name));                                                  \
                                                                                                   \
        for (i = 0; i < count; i++) {                                                              \
            values[i] = converter(rb_ary_entry(array, i));                                         \
        }                                                                                          \
                                                                                                   \
        sf_setter((sfShader*)Get_Shader_Struct(self), name, values, (size_t)count);                \
                                                                                                   \
        ALLOCV_END(buffer);                                                                        \
                                                                                                   \
        return self;                                                                               \
    }

static float shader_float_from_rb(VALUE rb_value) {
    return (float)NUM2DBL(rb_value);
}

SHADER_ARRAY_UNIFORM(float, float, shader_float_from_rb, sfShader_setFloatUniformArray)
SHADER_ARRAY_UNIFORM(vec2, sfGlslVec2, vec2f_from_rb, sfShader_setVec2UniformArray)
SHADER_ARRAY_UNIFORM(vec3, sfGlslVec3, vec3f_from_rb, sfShader_setVec3UniformArray)
SHADER_ARRAY_UNIFORM(vec4, sfGlslVec4, glsl_vec4_from_rb, sfShader_setVec4UniformArray)
SHADER_ARRAY_UNIFORM(mat3, sfGlslMat3, glsl_mat3_from_rb, sfShader_setMat3UniformArray)
SHADER_ARRAY_UNIFORM(mat4, sfGlslMat4, glsl_mat4_from_rb, sfShader_setMat4UniformArray)

#undef SHADER_ARRAY_UNIFORM

/* Document-method: SFML::Shader#set_texture
 * call-seq:
 *   set_texture(name, texture) -> Texture
 *
 * SFML keeps and re-binds a pointer to +texture+ internally (rather than
 * copying it) for as long as the shader may use it; this binding keeps a
 * reference alongside it so the Texture can't be GC'd out from under the
 * shader. CSFML's C API unconditionally dereferences the texture argument,
 * so unlike most other uniform setters here, +nil+ is not accepted.
 *
 * @return [Texture] +texture+
 * @raise [ArgumentError] if +texture+ is not a Texture
 */
static VALUE Shader_set_texture(VALUE self, VALUE rb_name, VALUE rb_texture) {
    Shader* wrapper;
    char name[256];

    TypedData_Get_Struct(self, Shader, &Shader_data_type, wrapper);
    Shader_name(rb_name, name, sizeof(name));

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    sfShader_setTextureUniform(wrapper->shader, name, Get_Texture_Struct(rb_texture));
    rb_hash_aset(wrapper->rb_textures, rb_str_new_cstr(name), rb_texture);

    return rb_texture;
}

/* call-seq:
 *   set_current_texture(name) -> self
 *
 * Binds the target's own currently-bound texture (CSFML's "current
 * texture" special value) to the sampler uniform +name+.
 *
 * @return [self]
 */
static VALUE Shader_set_current_texture(VALUE self, VALUE rb_name) {
    char name[256];

    Shader_name(rb_name, name, sizeof(name));
    sfShader_setCurrentTextureUniform((sfShader*)Get_Shader_Struct(self), name);

    return self;
}

/* call-seq:
 *   set_uniform(name, value) -> self
 *   uniform=(name, value)    -> self
 *
 * Sets the uniform +name+, dispatching to the appropriate +set_*+ method
 * based on +value+'s class: Color, Texture, Vector2, Vector3, true/false,
 * Integer, Float, or an Array (dispatched by length: 2 -> vec2, 3 -> vec3,
 * 4 -> vec4, 9 -> mat3, 16 -> mat4).
 *
 * @return [self]
 * @raise [ArgumentError] if +value+'s class/length isn't recognized
 */
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

/* Document-class: SFML::Shader
 * A GLSL vertex/geometry/fragment shader program, uploaded to the GPU and
 * carried on RenderState#shader.
 *
 * The +set_*_array+ methods below are generated from a shared macro (see
 * ext/graphics/shader.c) and documented here directly since the macro hides
 * their function bodies from the doc-comment scanner.
 *
 * @!method set_float_array(name, values)
 *   @return [self]
 * @!method set_vec2_array(name, values)
 *   @return [self]
 * @!method set_vec3_array(name, values)
 *   @return [self]
 * @!method set_vec4_array(name, values)
 *   values is an Array of 4-element Arrays.
 *   @return [self]
 * @!method set_mat3_array(name, values)
 *   values is an Array of 9-element Arrays.
 *   @return [self]
 * @!method set_mat4_array(name, values)
 *   values is an Array of 16-element Arrays.
 *   @return [self]
 */
void Init_Shader(VALUE rb_mSFML) {
    rb_cShader = rb_define_class_under(rb_mSFML, "Shader", rb_cObject);

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
    rb_define_method(rb_cShader, "set_vec4_array", Shader_set_vec4_array, 2);
    rb_define_method(rb_cShader, "set_mat3_array", Shader_set_mat3_array, 2);
    rb_define_method(rb_cShader, "set_mat4_array", Shader_set_mat4_array, 2);
    rb_define_method(rb_cShader, "set_texture", Shader_set_texture, 2);
    rb_define_method(rb_cShader, "set_current_texture", Shader_set_current_texture, 1);
    rb_define_method(rb_cShader, "set_uniform", Shader_set_uniform, 2);
    rb_define_method(rb_cShader, "uniform=", Shader_set_uniform, 2);
}

VALUE Get_Klass_Shader(void) {
    return rb_cShader;
}

const sfShader* Get_Shader_Struct(VALUE self) {
    Shader* ptr;
    TypedData_Get_Struct(self, Shader, &Shader_data_type, ptr);
    return ptr->shader;
}

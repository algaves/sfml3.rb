#include "graphics/render_state.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/transform.h"
#include "graphics/blend_mode.h"
#include "graphics/stencil_mode.h"
#include "graphics/enums.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "core/exceptions.h"
#include "core/macros.h"

/* The wrapped struct holds Ruby references (texture, shader) alongside the C
   states, so the data type needs a dmark. The C struct is a member rather than
   the whole allocation so those references have somewhere to live. */
typedef struct {
    sfRenderStates c;
    VALUE rb_texture;
    VALUE rb_shader;
} RenderStates;

static VALUE rb_cRenderState;

static void RenderStates_mark(void* ptr) {
    RenderStates* states = ptr;

    rb_gc_mark(states->rb_texture);
    rb_gc_mark(states->rb_shader);
}

static void RenderStates_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t RenderStates_data_type = {
    .wrap_struct_name = "SFML::RenderState",
    .function = {.dmark = RenderStates_mark, .dfree = RenderStates_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static RenderStates* RenderStates_create(void) {
    RenderStates* states = malloc(sizeof(RenderStates));

    /* Copy the defaults rather than filling fields by hand: CSFML 2 left the
       struct smaller, and spelling fields out is how stencilMode and
       coordinateType ended up uninitialised under CSFML 3. */
    states->c = sfRenderStates_default;
    states->rb_texture = Qnil;
    states->rb_shader = Qnil;

    return states;
}

static VALUE RenderStates_wrap(VALUE klass, RenderStates* states) {
    return TypedData_Wrap_Struct(klass, &RenderStates_data_type, states);
}

static VALUE RenderStates_alloc(VALUE klass) {
    return RenderStates_wrap(klass, RenderStates_create());
}

/* call-seq:
 *   RenderState.new         -> RenderState
 *   RenderState.new(matrix) -> RenderState
 *
 * Creates a render state with SFML's default blend mode, stencil mode and
 * coordinate type, and identity transform unless +matrix+ (a 3x3
 * row-major Array, see Transform) is given.
 *
 * @return [RenderState]
 * @raise [ArgumentError] if given more than one argument
 */
static VALUE RenderStates_initialize(int argc, VALUE* argv, VALUE self) {
    if (argc > 1) {
        raise_invalid_arguments_excepted(1, argc);
    }

    if (argc == 1) {
        Get_RenderState_Struct(self)->transform = Transform_ArrayToTransform(argv[0]);
    }

    return self;
}

/* call-seq:
 *   transform=(matrix) -> self
 *   matrix=(matrix) -> self
 *
 * +matrix+ is a 3x3 row-major Array, see Transform.
 *
 * @return [self]
 */
static VALUE RenderStates_set_transform(VALUE self, VALUE rb_matrix) {
    sfRenderStates* states = Get_RenderState_Struct(self);

    states->transform = Transform_ArrayToTransform(rb_matrix);

    return self;
}

/* call-seq: transform -> Array
 *
 * Also available as #matrix.
 *
 * @return [Array] the 3x3 row-major transform matrix
 */
static VALUE RenderStates_get_transform(VALUE self) {
    return Transform_MatrixToArray(Get_RenderState_Struct(self)->transform.matrix);
}

/* call-seq: blend_mode -> BlendMode
 *
 * Returns the state's blend mode.
 *
 * @return [BlendMode]
 */
static VALUE RenderStates_get_blend_mode(VALUE self) {
    return blend_mode_to_rb(Get_RenderState_Struct(self)->blendMode);
}

/* call-seq:
 *   blend_mode=(value) -> BlendMode
 *
 * Sets the state's blend mode.
 *
 * @return [BlendMode] +value+
 */
static VALUE RenderStates_set_blend_mode(VALUE self, VALUE rb_mode) {
    Get_RenderState_Struct(self)->blendMode = blend_mode_from_rb(rb_mode);
    return rb_mode;
}

/* call-seq: stencil_mode -> StencilMode
 *
 * Returns the state's stencil mode.
 *
 * @return [StencilMode]
 */
static VALUE RenderStates_get_stencil_mode(VALUE self) {
    return stencil_mode_to_rb(Get_RenderState_Struct(self)->stencilMode);
}

/* call-seq:
 *   stencil_mode=(value) -> StencilMode
 *
 * Sets the state's stencil mode.
 *
 * @return [StencilMode] +value+
 */
static VALUE RenderStates_set_stencil_mode(VALUE self, VALUE rb_mode) {
    Get_RenderState_Struct(self)->stencilMode = stencil_mode_from_rb(rb_mode);
    return rb_mode;
}

/* call-seq: coordinate_type -> Symbol
 *
 * Returns the state's texture coordinate type.
 *
 * @return [Symbol] either +:pixels+ or +:normalized+
 */
static VALUE RenderStates_get_coordinate_type(VALUE self) {
    return ID2SYM(rb_intern(coordinate_type_name(Get_RenderState_Struct(self)->coordinateType)));
}

/* call-seq:
 *   coordinate_type=(value) -> Symbol
 *
 * +value+ is +:pixels+ or +:normalized+.
 *
 * @return [Symbol] +value+
 */
static VALUE RenderStates_set_coordinate_type(VALUE self, VALUE rb_type) {
    Get_RenderState_Struct(self)->coordinateType = coordinate_type_from_rb(rb_type);
    return rb_type;
}

/* sfRenderStates is the first member, so its address is the wrapper's. */
static RenderStates* Get_RenderStates_Wrapper(VALUE self) {
    return (RenderStates*)Get_RenderState_Struct(self);
}

/* call-seq: texture -> Texture or nil
 *
 * Returns the object's texture, or +nil+ if it has none.
 *
 * @return [Texture, nil]
 */
static VALUE RenderStates_get_texture(VALUE self) {
    return Get_RenderStates_Wrapper(self)->rb_texture;
}

/* call-seq:
 *   texture=(value) -> Texture or nil
 *
 * Sets the object's texture.
 *
 * @return [Texture, nil] +value+
 */
static VALUE RenderStates_set_texture(VALUE self, VALUE rb_texture) {
    RenderStates* states = Get_RenderStates_Wrapper(self);

    if (NIL_P(rb_texture)) {
        states->rb_texture = Qnil;
        states->c.texture = NULL;
        return rb_texture;
    }

    if (!rb_obj_is_kind_of(rb_texture, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    states->rb_texture = rb_texture;
    states->c.texture = Get_Texture_Struct(rb_texture);

    return rb_texture;
}

/* call-seq: shader -> Shader or nil
 *
 * Returns the state's shader, or +nil+ if it has none.
 *
 * @return [Shader, nil]
 */
static VALUE RenderStates_get_shader(VALUE self) {
    return Get_RenderStates_Wrapper(self)->rb_shader;
}

/* call-seq:
 *   shader=(value) -> Shader or nil
 *
 * Sets the state's shader.
 *
 * @return [Shader, nil] +value+
 */
static VALUE RenderStates_set_shader(VALUE self, VALUE rb_shader) {
    RenderStates* states = Get_RenderStates_Wrapper(self);

    if (NIL_P(rb_shader)) {
        states->rb_shader = Qnil;
        states->c.shader = NULL;
        return rb_shader;
    }

    if (!rb_obj_is_kind_of(rb_shader, Get_Klass_Shader())) {
        raise_invalid_argument_class(Get_Klass_Shader());
    }

    states->rb_shader = rb_shader;
    states->c.shader = Get_Shader_Struct(rb_shader);

    return rb_shader;
}

/* Document-class: SFML::RenderState
 * The set of render states (transform, blend mode, stencil mode,
 * coordinate type, texture, shader) applied when drawing a Drawable to a
 * render target.
 *
 * @!attribute transform
 *   The object's transform matrix.
 *   @return [Array] the 3x3 row-major transform matrix
 * @!attribute blend_mode
 *   The blend mode applied to the draw.
 *   @return [BlendMode]
 * @!attribute stencil_mode
 *   The stencil mode applied to the draw.
 *   @return [StencilMode]
 * @!attribute coordinate_type
 *   The type of texture coordinates used.
 *   @return [Symbol] either +:pixels+ or +:normalized+
 * @!attribute texture
 *   The object's texture, or +nil+ if it has none.
 *   @return [Texture, nil]
 * @!attribute shader
 *   The shader applied to the draw, or +nil+ if none.
 *   @return [Shader, nil]
 */
void Init_RenderState(VALUE rb_mSFML) {
    rb_cRenderState = rb_define_class_under(rb_mSFML, "RenderState", rb_cObject);

    rb_define_alloc_func(rb_cRenderState, RenderStates_alloc);
    rb_define_method(rb_cRenderState, "initialize", RenderStates_initialize, -1);

    // setters
    rb_define_method(rb_cRenderState, "transform=", RenderStates_set_transform, 1);
    rb_define_method(rb_cRenderState, "matrix=", RenderStates_set_transform, 1);
    rb_define_method(rb_cRenderState, "blend_mode=", RenderStates_set_blend_mode, 1);
    rb_define_method(rb_cRenderState, "stencil_mode=", RenderStates_set_stencil_mode, 1);
    rb_define_method(rb_cRenderState, "coordinate_type=", RenderStates_set_coordinate_type, 1);
    rb_define_method(rb_cRenderState, "texture=", RenderStates_set_texture, 1);
    rb_define_method(rb_cRenderState, "shader=", RenderStates_set_shader, 1);

    // getters
    rb_define_method(rb_cRenderState, "transform", RenderStates_get_transform, 0);
    rb_define_method(rb_cRenderState, "matrix", RenderStates_get_transform, 0);
    rb_define_method(rb_cRenderState, "blend_mode", RenderStates_get_blend_mode, 0);
    rb_define_method(rb_cRenderState, "stencil_mode", RenderStates_get_stencil_mode, 0);
    rb_define_method(rb_cRenderState, "coordinate_type", RenderStates_get_coordinate_type, 0);
    rb_define_method(rb_cRenderState, "texture", RenderStates_get_texture, 0);
    rb_define_method(rb_cRenderState, "shader", RenderStates_get_shader, 0);
}

sfRenderStates* Get_RenderState_Struct(VALUE self) {
    RenderStates* states;
    TypedData_Get_Struct(self, RenderStates, &RenderStates_data_type, states);
    return &states->c;
}

VALUE Get_Klass_RenderState(void) {
    return rb_cRenderState;
}

VALUE Get_New_RenderState(void) {
    return RenderStates_wrap(Get_Klass_RenderState(), RenderStates_create());
}

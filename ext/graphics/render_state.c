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

static void RenderStates_mark(void *ptr) {
    RenderStates *states = ptr;

    rb_gc_mark(states->rb_texture);
    rb_gc_mark(states->rb_shader);
}

static void RenderStates_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t RenderStates_data_type = {
    .wrap_struct_name = "SFML::RenderState",
    .function = {.dmark = RenderStates_mark, .dfree = RenderStates_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static RenderStates *RenderStates_create(void) {
    RenderStates *states = malloc(sizeof(RenderStates));

    /* Copy the defaults rather than filling fields by hand: CSFML 2 left the
       struct smaller, and spelling fields out is how stencilMode and
       coordinateType ended up uninitialised under CSFML 3. */
    states->c = sfRenderStates_default;
    states->rb_texture = Qnil;
    states->rb_shader = Qnil;

    return states;
}

static VALUE RenderStates_wrap(VALUE klass, RenderStates *states) {
    return TypedData_Wrap_Struct(klass, &RenderStates_data_type, states);
}

static VALUE RenderStates_new(int argc, VALUE *argv, VALUE klass) {
    RenderStates *states;
    VALUE self;

    if (argc > 1) {
        raise_invalid_arguments_excepted(1, argc);
    }

    states = RenderStates_create();

    if (argc == 1) {
        Transform_SwapMatrix(Transform_ArrayToTransform(argv[0]).matrix, states->c.transform.matrix);
    }

    self = RenderStates_wrap(klass, states);

    return self;
}

static VALUE RenderStates_set_transform(VALUE self, VALUE rb_matrix) {
    sfRenderStates *states = Get_RenderState_Struct(self);

    Transform_SwapMatrix(Transform_ArrayToTransform(rb_matrix).matrix, states->transform.matrix);

    return self;
}

static VALUE RenderStates_get_transform(VALUE self) {
    return Transform_MatrixToArray(Get_RenderState_Struct(self)->transform.matrix);
}

static VALUE RenderStates_get_blend_mode(VALUE self) {
    return blend_mode_to_rb(Get_RenderState_Struct(self)->blendMode);
}

static VALUE RenderStates_set_blend_mode(VALUE self, VALUE rb_mode) {
    Get_RenderState_Struct(self)->blendMode = blend_mode_from_rb(rb_mode);
    return rb_mode;
}

static VALUE RenderStates_get_stencil_mode(VALUE self) {
    return stencil_mode_to_rb(Get_RenderState_Struct(self)->stencilMode);
}

static VALUE RenderStates_set_stencil_mode(VALUE self, VALUE rb_mode) {
    Get_RenderState_Struct(self)->stencilMode = stencil_mode_from_rb(rb_mode);
    return rb_mode;
}

static VALUE RenderStates_get_coordinate_type(VALUE self) {
    return ID2SYM(rb_intern(coordinate_type_name(Get_RenderState_Struct(self)->coordinateType)));
}

static VALUE RenderStates_set_coordinate_type(VALUE self, VALUE rb_type) {
    Get_RenderState_Struct(self)->coordinateType = coordinate_type_from_rb(rb_type);
    return rb_type;
}

/* sfRenderStates is the first member, so its address is the wrapper's. */
static RenderStates *Get_RenderStates_Wrapper(VALUE self) {
    return (RenderStates *) Get_RenderState_Struct(self);
}

static VALUE RenderStates_get_texture(VALUE self) {
    return Get_RenderStates_Wrapper(self)->rb_texture;
}

static VALUE RenderStates_set_texture(VALUE self, VALUE rb_texture) {
    RenderStates *states = Get_RenderStates_Wrapper(self);

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

static VALUE RenderStates_get_shader(VALUE self) {
    return Get_RenderStates_Wrapper(self)->rb_shader;
}

static VALUE RenderStates_set_shader(VALUE self, VALUE rb_shader) {
    RenderStates *states = Get_RenderStates_Wrapper(self);

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

void Init_RenderState(VALUE rb_module) {
    rb_cRenderState = rb_define_class_under(rb_module, "RenderState", rb_cObject);

    rb_define_singleton_method(rb_cRenderState, "new", RenderStates_new, -1);

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

sfRenderStates *Get_RenderState_Struct(VALUE self) {
    RenderStates *states;
    TypedData_Get_Struct(self, RenderStates, &RenderStates_data_type, states);
    return &states->c;
}

VALUE Get_Klass_RenderState(void) {
    return rb_cRenderState;
}

VALUE Get_New_RenderState(void) {
    return RenderStates_wrap(Get_Klass_RenderState(), RenderStates_create());
}

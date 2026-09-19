#include "graphics/vertex_buffer.h"

#include <ruby.h>
#include <stdlib.h>
#include <string.h>

#include "graphics/drawable.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/vertex.h"
#include "graphics/enums.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cVertexBuffer;

static const char* usage_names[] = {"stream", "dynamic", "static"};

static void VertexBuffer_free(void* ptr) {
    if (ptr != NULL) {
        sfVertexBuffer_destroy(ptr);
    }
}

static const rb_data_type_t VertexBuffer_data_type = {
    .wrap_struct_name = "SFML::VertexBuffer",
    .function = {.dmark = NULL, .dfree = VertexBuffer_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE VertexBuffer_wrap(VALUE klass, sfVertexBuffer* buffer) {
    if (buffer == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create vertex buffer");
    }

    return TypedData_Wrap_Struct(klass, &VertexBuffer_data_type, buffer);
}

sfVertexBuffer* Get_VertexBuffer_Struct(VALUE self) {
    sfVertexBuffer* ptr;
    TypedData_Get_Struct(self, sfVertexBuffer, &VertexBuffer_data_type, ptr);
    return ptr;
}

static sfVertexBufferUsage usage_from_rb(VALUE rb_usage) {
    size_t i;

    if (RB_INTEGER_TYPE_P(rb_usage)) {
        return (sfVertexBufferUsage)NUM2INT(rb_usage);
    }

    if (SYMBOL_P(rb_usage)) {
        const char* name = rb_id2name(SYM2ID(rb_usage));

        for (i = 0; i < 3; i++) {
            if (strcmp(name, usage_names[i]) == 0) {
                return (sfVertexBufferUsage)i;
            }
        }
    }

    rb_raise(rb_eArgError, "unknown vertex buffer usage");
    return sfVertexBufferStatic;
}

static VALUE VertexBuffer_initialize_copy(VALUE self, VALUE other) {
    (void)other;
    rb_raise(rb_eTypeError, "can't copy a %s", rb_obj_classname(self));
}

static VALUE VertexBuffer_alloc(VALUE klass) {
    return TypedData_Wrap_Struct(klass, &VertexBuffer_data_type, NULL);
}

/* call-seq:
 *   VertexBuffer.new                                    -> VertexBuffer
 *   VertexBuffer.new(count)                             -> VertexBuffer
 *   VertexBuffer.new(count, primitive)                  -> VertexBuffer
 *   VertexBuffer.new(count, primitive, usage)            -> VertexBuffer
 *
 * Creates a GPU-side vertex buffer holding +count+ vertices (default 0),
 * interpreted as +primitive+ (default :points, see VertexArray#primitive),
 * with the given +usage+ hint (one of :stream, :dynamic, :static; default
 * :static).
 *
 * @return [VertexBuffer]
 * @raise [RuntimeError] if creation fails
 */
static VALUE VertexBuffer_initialize(int argc, VALUE* argv, VALUE self) {
    VALUE rb_count, rb_primitive, rb_usage;
    size_t count = 0;
    sfPrimitiveType primitive = sfPoints;
    sfVertexBufferUsage usage = sfVertexBufferStatic;
    sfVertexBuffer* buffer;

    rb_scan_args(argc, argv, "03", &rb_count, &rb_primitive, &rb_usage);

    if (!NIL_P(rb_count)) {
        count = (size_t)NUM2SIZET(rb_count);
    }

    if (!NIL_P(rb_primitive)) {
        primitive = primitive_type_from_rb(rb_primitive);
    }

    if (!NIL_P(rb_usage)) {
        usage = usage_from_rb(rb_usage);
    }

    buffer = sfVertexBuffer_create(count, primitive, usage);

    if (buffer == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create vertex buffer");
    }

    DATA_PTR(self) = buffer;

    return self;
}

/* call-seq: copy -> VertexBuffer
 *
 * Returns a deep copy of the object.
 *
 * @return [VertexBuffer] an independent copy
 */
static VALUE VertexBuffer_copy(VALUE self) {
    return VertexBuffer_wrap(Get_Klass_VertexBuffer(),
                             sfVertexBuffer_copy(Get_VertexBuffer_Struct(self)));
}

/* call-seq: vertex_count -> Integer
 *
 * Returns the number of vertices stored.
 *
 * @return [Integer]
 */
static VALUE VertexBuffer_get_vertex_count(VALUE self) {
    return SIZET2NUM(sfVertexBuffer_getVertexCount(Get_VertexBuffer_Struct(self)));
}

/* call-seq:
 *   update(vertices)         -> true or false
 *   update(vertices, offset) -> true or false
 *
 * Uploads +vertices+ (an Array of Vertex, or of anything Vertex.new
 * accepts) into the buffer starting at +offset+ (default 0). The buffer
 * must have enough room; use #resize (via .new) if not.
 *
 * @return [Boolean] whether the update succeeded
 */
static VALUE VertexBuffer_update(int argc, VALUE* argv, VALUE self) {
    VALUE rb_vertices, rb_offset;
    unsigned int offset = 0;
    size_t count;
    sfVertex* vertices;
    bool result;

    rb_scan_args(argc, argv, "11", &rb_vertices, &rb_offset);

    if (!NIL_P(rb_offset)) {
        offset = (unsigned int)NUM2UINT(rb_offset);
    }

    vertices = vertices_from_rb(rb_vertices, &count);

    result =
        sfVertexBuffer_update(Get_VertexBuffer_Struct(self), vertices, (unsigned int)count, offset);

    xfree(vertices);

    return BOOL2RB(result);
}

/* call-seq:
 *   update_from(other) -> true or false
 *
 * Copies the contents of +other+ (a VertexBuffer of the same vertex count)
 * into this buffer.
 *
 * @return [Boolean] whether the update succeeded
 * @raise [TypeError] if +other+ is not a VertexBuffer
 */
static VALUE VertexBuffer_update_from(VALUE self, VALUE rb_other) {
    if (!rb_obj_is_kind_of(rb_other, rb_cVertexBuffer)) {
        raise_invalid_argument_class(rb_cVertexBuffer);
    }

    return BOOL2RB(sfVertexBuffer_updateFromVertexBuffer(Get_VertexBuffer_Struct(self),
                                                         Get_VertexBuffer_Struct(rb_other)));
}

/* Exchanges the two buffers' contents in place, so anything already holding
   either object sees the swap. */
/* call-seq:
 *   swap(other) -> self
 *
 * Swaps the contents with +other+.
 *
 * @return [self]
 * @raise [TypeError] if +other+ is not a VertexBuffer
 */
static VALUE VertexBuffer_swap(VALUE self, VALUE rb_other) {
    if (!rb_obj_is_kind_of(rb_other, rb_cVertexBuffer)) {
        raise_invalid_argument_class(rb_cVertexBuffer);
    }

    sfVertexBuffer_swap(Get_VertexBuffer_Struct(self), Get_VertexBuffer_Struct(rb_other));

    return self;
}

/* call-seq: primitive -> Symbol
 *
 * Returns the primitive type used to draw the vertices.
 *
 * @return [Symbol] see VertexArray#primitive
 */
static VALUE VertexBuffer_get_primitive_type(VALUE self) {
    return ID2SYM(rb_intern(
        primitive_type_name(sfVertexBuffer_getPrimitiveType(Get_VertexBuffer_Struct(self)))));
}

/* call-seq:
 *   primitive=(value) -> Symbol
 *
 * Sets the primitive type used to draw the vertices.
 *
 * @return [Symbol] +value+
 */
static VALUE VertexBuffer_set_primitive_type(VALUE self, VALUE rb_type) {
    sfVertexBuffer_setPrimitiveType(Get_VertexBuffer_Struct(self), primitive_type_from_rb(rb_type));
    return rb_type;
}

/* call-seq: usage -> Symbol
 *
 * Returns the buffer's usage hint.
 *
 * @return [Symbol] one of :stream, :dynamic, :static
 */
static VALUE VertexBuffer_get_usage(VALUE self) {
    return ID2SYM(rb_intern(usage_names[sfVertexBuffer_getUsage(Get_VertexBuffer_Struct(self))]));
}

/* call-seq:
 *   usage=(value) -> Symbol
 *
 * Sets the buffer's usage hint.
 *
 * @return [Symbol] +value+
 */
static VALUE VertexBuffer_set_usage(VALUE self, VALUE rb_usage) {
    sfVertexBuffer_setUsage(Get_VertexBuffer_Struct(self), usage_from_rb(rb_usage));
    return rb_usage;
}

/* call-seq: native_handle -> Integer
 *
 * Returns the underlying OpenGL handle.
 *
 * @return [Integer] the underlying OpenGL buffer handle
 */
static VALUE VertexBuffer_get_native_handle(VALUE self) {
    return UINT2NUM(sfVertexBuffer_getNativeHandle(Get_VertexBuffer_Struct(self)));
}

/* call-seq: bind -> self
 *
 * Binds the buffer for drawing.
 *
 * @return [self]
 */
static VALUE VertexBuffer_bind(VALUE self) {
    sfVertexBuffer_bind(Get_VertexBuffer_Struct(self));
    return self;
}

/* call-seq:
 *   VertexBuffer.available? -> true or false
 *
 * Returns +true+ if vertex buffers are supported.
 *
 * @return [Boolean] whether the system supports vertex buffers
 */
static VALUE VertexBuffer_is_available(VALUE klass) {
    return BOOL2RB(sfVertexBuffer_isAvailable());
}

/* call-seq:
 *   draw(target, state) -> nil
 *
 * Part of the Drawable interface; call Target#draw instead of this
 * directly.
 *
 * @return [nil]
 * @raise [TypeError] if +target+ is not a Target or +state+ is not a
 *   RenderState
 */
static VALUE VertexBuffer_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawVertexBuffer,
                sfRenderTexture_drawVertexBuffer, Get_VertexBuffer_Struct(self),
                Get_RenderState_Struct(rb_state));

    return Qnil;
}

/* Document-class: SFML::VertexBuffer
 * Like VertexArray, but the vertex data lives in GPU memory rather than
 * being re-uploaded on every draw -- more efficient for large, static
 * or semi-static vertex sets.
 *
 * Includes +Drawable+.
 *
 * @!attribute primitive
 *   The primitive type used to draw the vertices.
 *   @return [Symbol]
 * @!attribute usage
 *   The buffer's usage hint.
 *   @return [Symbol] one of :stream, :dynamic, :static
 */
void Init_VertexBuffer(VALUE rb_mSFML) {
    rb_cVertexBuffer = rb_define_class_under(rb_mSFML, "VertexBuffer", rb_cObject);

    rb_define_alloc_func(rb_cVertexBuffer, VertexBuffer_alloc);

    rb_include_module(rb_cVertexBuffer, Get_Module_Drawable());

    rb_define_method(rb_cVertexBuffer, "initialize", VertexBuffer_initialize, -1);
    rb_define_private_method(rb_cVertexBuffer, "initialize_copy", VertexBuffer_initialize_copy, 1);
    rb_define_singleton_method(rb_cVertexBuffer, "available?", VertexBuffer_is_available, 0);

    rb_define_method(rb_cVertexBuffer, "copy", VertexBuffer_copy, 0);
    rb_define_method(rb_cVertexBuffer, "vertex_count", VertexBuffer_get_vertex_count, 0);
    rb_define_method(rb_cVertexBuffer, "update", VertexBuffer_update, -1);
    rb_define_method(rb_cVertexBuffer, "update_from", VertexBuffer_update_from, 1);
    rb_define_method(rb_cVertexBuffer, "swap", VertexBuffer_swap, 1);
    rb_define_method(rb_cVertexBuffer, "primitive", VertexBuffer_get_primitive_type, 0);
    rb_define_method(rb_cVertexBuffer, "primitive=", VertexBuffer_set_primitive_type, 1);
    rb_define_method(rb_cVertexBuffer, "usage", VertexBuffer_get_usage, 0);
    rb_define_method(rb_cVertexBuffer, "usage=", VertexBuffer_set_usage, 1);
    rb_define_method(rb_cVertexBuffer, "native_handle", VertexBuffer_get_native_handle, 0);
    rb_define_method(rb_cVertexBuffer, "bind", VertexBuffer_bind, 0);
    rb_define_method(rb_cVertexBuffer, "draw", VertexBuffer_draw, 2);
}

VALUE Get_Klass_VertexBuffer(void) {
    return rb_cVertexBuffer;
}

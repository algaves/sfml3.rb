#include "graphics/vertex_array.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/drawable.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/rect.h"
#include "graphics/vertex.h"
#include "graphics/enums.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cVertexArray;

static void VertexArray_free(void* ptr) {
    sfVertexArray_destroy(ptr);
}

static const rb_data_type_t VertexArray_data_type = {
    .wrap_struct_name = "SFML::VertexArray",
    .function = {.dmark = NULL, .dfree = VertexArray_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE VertexArray_wrap(VALUE klass, sfVertexArray* array) {
    if (array == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create vertex array");
    }

    return TypedData_Wrap_Struct(klass, &VertexArray_data_type, array);
}

static sfVertexArray* Get_VertexArray_Struct(VALUE self) {
    sfVertexArray* ptr;
    TypedData_Get_Struct(self, sfVertexArray, &VertexArray_data_type, ptr);
    return ptr;
}

/* call-seq:
 *   VertexArray.new -> VertexArray
 *
 * @return [VertexArray] an empty array of :points primitives
 */
static VALUE VertexArray_new(VALUE klass) {
    return VertexArray_wrap(klass, sfVertexArray_create());
}

/* call-seq: copy -> VertexArray
 *
 * @return [VertexArray] an independent copy
 */
static VALUE VertexArray_copy(VALUE self) {
    return VertexArray_wrap(Get_Klass_VertexArray(),
                            sfVertexArray_copy(Get_VertexArray_Struct(self)));
}

/* call-seq: vertex_count -> Integer
 *
 * @return [Integer]
 */
static VALUE VertexArray_get_vertex_count(VALUE self) {
    return SIZET2NUM(sfVertexArray_getVertexCount(Get_VertexArray_Struct(self)));
}

static size_t VertexArray_check_index(VALUE self, VALUE rb_index) {
    void* array = Get_VertexArray_Struct(self);
    size_t index = (size_t)NUM2SIZET(rb_index);
    size_t count = sfVertexArray_getVertexCount(array);

    if (index >= count) {
        rb_raise(rb_eIndexError, "index %zu outside of vertex count %zu", index, count);
    }

    return index;
}

/* call-seq:
 *   vertex(index) -> Vertex
 *
 * @return [Vertex] a copy of the vertex at +index+
 * @raise [IndexError] if +index+ is out of range
 */
static VALUE VertexArray_get_vertex(VALUE self, VALUE rb_index) {
    return vertex_to_rb(*sfVertexArray_getVertex(Get_VertexArray_Struct(self),
                                                 VertexArray_check_index(self, rb_index)));
}

/* call-seq:
 *   set_vertex(index, vertex) -> Vertex
 *
 * @return [Vertex] +vertex+
 * @raise [IndexError] if +index+ is out of range
 */
static VALUE VertexArray_set_vertex(VALUE self, VALUE rb_index, VALUE rb_vertex) {
    *sfVertexArray_getVertex(Get_VertexArray_Struct(self),
                             VertexArray_check_index(self, rb_index)) = vertex_from_rb(rb_vertex);
    return rb_vertex;
}

/* call-seq:
 *   append(vertex) -> self
 *
 * @return [self]
 */
static VALUE VertexArray_append(VALUE self, VALUE rb_vertex) {
    sfVertexArray_append(Get_VertexArray_Struct(self), vertex_from_rb(rb_vertex));
    return self;
}

/* call-seq: clear! -> self
 *
 * Removes all vertices.
 *
 * @return [self]
 */
static VALUE VertexArray_clear(VALUE self) {
    sfVertexArray_clear(Get_VertexArray_Struct(self));
    return self;
}

/* call-seq:
 *   resize(count) -> self
 *
 * Grows or shrinks the array to +count+ vertices; new vertices are default
 * ones.
 *
 * @return [self]
 */
static VALUE VertexArray_resize(VALUE self, VALUE rb_count) {
    sfVertexArray_resize(Get_VertexArray_Struct(self), (size_t)NUM2SIZET(rb_count));
    return self;
}

/* call-seq: primitive -> Symbol
 *
 * @return [Symbol] the primitive type vertices are interpreted as (e.g.
 *   :points, :lines, :triangles)
 */
static VALUE VertexArray_get_primitive_type(VALUE self) {
    return ID2SYM(rb_intern(
        primitive_type_name(sfVertexArray_getPrimitiveType(Get_VertexArray_Struct(self)))));
}

/* call-seq:
 *   primitive=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE VertexArray_set_primitive_type(VALUE self, VALUE rb_type) {
    sfVertexArray_setPrimitiveType(Get_VertexArray_Struct(self), primitive_type_from_rb(rb_type));
    return rb_type;
}

/* call-seq: bounds -> Rect
 *
 * @return [Rect] the axis-aligned bounding box of all vertices
 */
static VALUE VertexArray_get_bounds(VALUE self) {
    return rect_to_rb(sfVertexArray_getBounds(Get_VertexArray_Struct(self)));
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
static VALUE VertexArray_draw(VALUE self, VALUE rb_target, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_target, Get_Klass_Target())) {
        raise_invalid_argument_class(Get_Klass_Target());
    }

    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    TARGET_DRAW(Get_Target_Struct(rb_target), sfRenderWindow_drawVertexArray,
                sfRenderTexture_drawVertexArray, Get_VertexArray_Struct(self),
                Get_RenderState_Struct(rb_state));

    return Qnil;
}

/* Document-class: SFML::VertexArray
 * A resizable, drawable set of Vertex objects interpreted as a given
 * primitive type (points, lines, triangles, ...).
 *
 * Includes +Drawable+.
 *
 * @!attribute primitive
 *   @return [Symbol]
 */
void Init_VertexArray(VALUE rb_mSFML) {
    rb_cVertexArray = rb_define_class_under(rb_mSFML, "VertexArray", rb_cObject);

    rb_include_module(rb_cVertexArray, Get_Module_Drawable());

    rb_define_singleton_method(rb_cVertexArray, "new", VertexArray_new, 0);

    rb_define_method(rb_cVertexArray, "copy", VertexArray_copy, 0);
    rb_define_method(rb_cVertexArray, "vertex_count", VertexArray_get_vertex_count, 0);
    rb_define_method(rb_cVertexArray, "vertex", VertexArray_get_vertex, 1);
    rb_define_method(rb_cVertexArray, "set_vertex", VertexArray_set_vertex, 2);
    rb_define_method(rb_cVertexArray, "append", VertexArray_append, 1);
    rb_define_method(rb_cVertexArray, "clear!", VertexArray_clear, 0);
    rb_define_method(rb_cVertexArray, "resize", VertexArray_resize, 1);
    rb_define_method(rb_cVertexArray, "primitive", VertexArray_get_primitive_type, 0);
    rb_define_method(rb_cVertexArray, "primitive=", VertexArray_set_primitive_type, 1);
    rb_define_method(rb_cVertexArray, "bounds", VertexArray_get_bounds, 0);
    rb_define_method(rb_cVertexArray, "draw", VertexArray_draw, 2);
}

VALUE Get_Klass_VertexArray(void) {
    return rb_cVertexArray;
}

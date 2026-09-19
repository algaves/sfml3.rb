#include "graphics/vertex.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/color.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfVertex vertex;
} Vertex;

static VALUE rb_cVertex;

static void Vertex_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Vertex_data_type = {
    .wrap_struct_name = "SFML::Vertex",
    .function = {.dmark = NULL, .dfree = Vertex_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Vertex_wrap(sfVertex vertex) {
    Vertex* ptr = malloc(sizeof(Vertex));

    ptr->vertex = vertex;

    return TypedData_Wrap_Struct(rb_cVertex, &Vertex_data_type, ptr);
}

/* call-seq:
 *   Vertex.new                              -> Vertex
 *   Vertex.new(position)                    -> Vertex
 *   Vertex.new(position, color)             -> Vertex
 *   Vertex.new(position, color, tex_coords) -> Vertex
 *
 * Any omitted argument defaults to position +(0, 0)+, color white, and
 * tex_coords +(0, 0)+.
 *
 * @return [Vertex]
 */
static VALUE Vertex_new(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_position, rb_color, rb_tex_coords;
    sfVertex vertex = {{0, 0}, {255, 255, 255, 255}, {0, 0}};

    rb_scan_args(argc, argv, "03", &rb_position, &rb_color, &rb_tex_coords);

    if (!NIL_P(rb_position)) {
        vertex.position = vec2f_from_rb(rb_position);
    }

    if (!NIL_P(rb_color)) {
        vertex.color = color_from_rb(rb_color);
    }

    if (!NIL_P(rb_tex_coords)) {
        vertex.texCoords = vec2f_from_rb(rb_tex_coords);
    }

    return Vertex_wrap(vertex);
}

/* call-seq: position -> Vector2
 *
 * @return [Vector2]
 */
static VALUE Vertex_get_position(VALUE self) {
    return vec2f_to_rb(((Vertex*)Get_Vertex_Struct(self))->vertex.position);
}

/* call-seq:
 *   position=(value) -> Vector2
 *
 * @return [Vector2] +value+
 */
static VALUE Vertex_set_position(VALUE self, VALUE rb_position) {
    ((Vertex*)Get_Vertex_Struct(self))->vertex.position = vec2f_from_rb(rb_position);
    return rb_position;
}

/* call-seq: color -> Color
 *
 * @return [Color]
 */
static VALUE Vertex_get_color(VALUE self) {
    return color_to_rb(((Vertex*)Get_Vertex_Struct(self))->vertex.color);
}

/* call-seq:
 *   color=(value) -> Color
 *
 * @return [Color] +value+
 */
static VALUE Vertex_set_color(VALUE self, VALUE rb_color) {
    ((Vertex*)Get_Vertex_Struct(self))->vertex.color = color_from_rb(rb_color);
    return rb_color;
}

/* call-seq: tex_coords -> Vector2
 *
 * @return [Vector2]
 */
static VALUE Vertex_get_tex_coords(VALUE self) {
    return vec2f_to_rb(((Vertex*)Get_Vertex_Struct(self))->vertex.texCoords);
}

/* call-seq:
 *   tex_coords=(value) -> Vector2
 *
 * @return [Vector2] +value+
 */
static VALUE Vertex_set_tex_coords(VALUE self, VALUE rb_tex_coords) {
    ((Vertex*)Get_Vertex_Struct(self))->vertex.texCoords = vec2f_from_rb(rb_tex_coords);
    return rb_tex_coords;
}

/* call-seq: to_a -> [Vector2, Color]
 *
 * @return [Array] +[position, color]+
 */
static VALUE Vertex_to_a(VALUE self) {
    Vertex* v = Get_Vertex_Struct(self);

    return rb_ary_new_from_args(2, vec2f_to_rb(v->vertex.position), color_to_rb(v->vertex.color));
}

/* call-seq:
 *   self == other -> true or false
 *
 * @return [Boolean]
 */
static VALUE Vertex_eql(VALUE self, VALUE rb_other) {
    sfVertex a = ((Vertex*)Get_Vertex_Struct(self))->vertex;
    sfVertex b;

    if (!rb_obj_is_kind_of(rb_other, rb_cVertex)) {
        return Qfalse;
    }

    b = ((Vertex*)Get_Vertex_Struct(rb_other))->vertex;

    return BOOL2RB(a.position.x == b.position.x && a.position.y == b.position.y &&
                   a.color.r == b.color.r && a.color.g == b.color.g && a.color.b == b.color.b &&
                   a.color.a == b.color.a && a.texCoords.x == b.texCoords.x &&
                   a.texCoords.y == b.texCoords.y);
}

/* Document-class: SFML::Vertex
 * A single point used to build up VertexArray and VertexBuffer primitives:
 * a position, a color, and texture coordinates.
 *
 * @!attribute position
 *   @return [Vector2]
 * @!attribute color
 *   @return [Color]
 * @!attribute tex_coords
 *   @return [Vector2]
 */
void Init_Vertex(VALUE rb_mSFML) {
    rb_cVertex = rb_define_class_under(rb_mSFML, "Vertex", rb_cObject);

    rb_define_singleton_method(rb_cVertex, "new", Vertex_new, -1);

    rb_define_method(rb_cVertex, "position", Vertex_get_position, 0);
    rb_define_method(rb_cVertex, "color", Vertex_get_color, 0);
    rb_define_method(rb_cVertex, "tex_coords", Vertex_get_tex_coords, 0);
    rb_define_method(rb_cVertex, "position=", Vertex_set_position, 1);
    rb_define_method(rb_cVertex, "color=", Vertex_set_color, 1);
    rb_define_method(rb_cVertex, "tex_coords=", Vertex_set_tex_coords, 1);
    rb_define_method(rb_cVertex, "to_a", Vertex_to_a, 0);
    rb_define_method(rb_cVertex, "==", Vertex_eql, 1);
}

VALUE Get_Klass_Vertex(void) {
    return rb_cVertex;
}

void* Get_Vertex_Struct(VALUE self) {
    Vertex* ptr;
    TypedData_Get_Struct(self, Vertex, &Vertex_data_type, ptr);
    return ptr;
}

sfVertex vertex_from_rb(VALUE rb_vertex) {
    if (rb_obj_is_kind_of(rb_vertex, rb_cVertex)) {
        return ((Vertex*)Get_Vertex_Struct(rb_vertex))->vertex;
    }

    if (RB_TYPE_P(rb_vertex, T_ARRAY)) {
        return vertex_from_rb(rb_funcall(rb_cVertex, rb_intern("new"), 1, rb_vertex));
    }

    raise_invalid_argument_class(rb_cVertex);

    return (sfVertex){{0, 0}, {0, 0, 0, 0}, {0, 0}};
}

VALUE vertex_to_rb(sfVertex c_vertex) {
    return Vertex_wrap(c_vertex);
}

sfVertex* vertices_from_rb(VALUE rb_vertices, size_t* count) {
    VALUE normalized;
    sfVertex* vertices;
    long i, len;

    Check_Type(rb_vertices, T_ARRAY);

    len = RARRAY_LEN(rb_vertices);
    normalized = rb_ary_new_capa(len);

    /* Normalize -- and so raise on a bad element -- before allocating anything:
       vertex_from_rb builds a Vertex out of a plain Array and can raise, and a
       raise after ALLOC_N would leak the buffer. */
    for (i = 0; i < len; i++) {
        VALUE entry = rb_ary_entry(rb_vertices, i);

        if (!rb_obj_is_kind_of(entry, rb_cVertex)) {
            entry = rb_funcall(rb_cVertex, rb_intern("new"), 1, entry);
        }

        rb_ary_push(normalized, entry);
    }

    vertices = ALLOC_N(sfVertex, len);

    for (i = 0; i < len; i++) {
        vertices[i] = vertex_from_rb(rb_ary_entry(normalized, i));
    }

    *count = (size_t)len;

    return vertices;
}

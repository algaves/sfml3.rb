#ifndef SFML_RB_GRAPHICS_VERTEX_H
#define SFML_RB_GRAPHICS_VERTEX_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Vertex(VALUE rb_module);

VALUE Get_Klass_Vertex(void);

void* Get_Vertex_Struct(VALUE self);

sfVertex vertex_from_rb(VALUE rb_vertex);

VALUE vertex_to_rb(sfVertex c_vertex);

/* Converts an Array of Vertex into a freshly allocated C array; the caller
   xfree()s it. Every element is validated before anything is allocated, so a
   bad element raises without leaking. */
sfVertex* vertices_from_rb(VALUE rb_vertices, size_t* count);

#endif // SFML_RB_GRAPHICS_VERTEX_H

#ifndef SFML_RB_SYSTEM_INPUT_STREAM_H
#define SFML_RB_SYSTEM_INPUT_STREAM_H

#include <ruby.h>

#include "core/sfml.h"

void Init_InputStream(VALUE rb_module);

VALUE Get_Klass_InputStream(void);

sfInputStream* Get_InputStream_Struct(VALUE self);

/* True when the object is an SF::System::InputStream. */
int InputStream_is_stream(VALUE rb_stream);

/* Returns the underlying sfInputStream, wrapping any #read-able object in an
   SF::System::InputStream first. When wrapping happens the wrapper is stored in
   *holder (a caller-local VALUE), so a GC during the subsequent load cannot
   collect it. */
sfInputStream* input_stream_from_rb(VALUE rb_stream, VALUE* holder);

#endif // SFML_RB_SYSTEM_INPUT_STREAM_H

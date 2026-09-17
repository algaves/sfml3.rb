#ifndef SFML_RB_CORE_UNICODE_H
#define SFML_RB_CORE_UNICODE_H

#include <ruby.h>

#include "core/sfml.h"

/* CSFML's plain `const char*` entry points hand the bytes to sf::String's
   narrow-character constructor, which decodes them with the C locale -- so a
   UTF-8 Ruby String arrives mangled. The *Unicode* entry points take UTF-32
   instead, which is lossless, and these two functions are the bridge.

   utf32_from_rb returns a hidden Ruby String owning the code units; read it
   with UTF32_PTR and keep the VALUE alive (RB_GC_GUARD) until the sf* call has
   returned. Ruby frees it, so an exception in between leaks nothing.

   Deliberately not ALLOCV: that allocates on the stack for small sizes, so its
   pointer cannot outlive the frame that created it. */
VALUE utf32_from_rb(VALUE rb_string);

#define UTF32_PTR(buffer) ((const sfChar32*)RSTRING_PTR(buffer))

VALUE utf32_to_rb(const sfChar32* string);

#endif // SFML_RB_CORE_UNICODE_H

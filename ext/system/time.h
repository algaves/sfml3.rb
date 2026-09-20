#ifndef SFML_RB_SYSTEM_TIME_H
#define SFML_RB_SYSTEM_TIME_H

#include <ruby.h>

#include "core/sfml.h"

// The Emscripten/WebAssembly port statically links this extension into
// ruby.wasm, so every exported symbol shares a namespace with Ruby's own
// static core. Ruby's time.c already defines Init_Time (for the Time class),
// which would clash with the SFML::Time binding under a static link; give it
// a wasm-unique name only there. Host builds keep the plain Init_* naming.
#ifdef SFML_RB_WASM
#define Init_Time Init_SFMLRB_Time
#endif

void Init_Time(VALUE rb_module);

VALUE Get_Klass_Time(void);

void* Get_Time_Struct(VALUE self);

sfTime time_from_rb(VALUE rb_time);

VALUE time_to_rb(sfTime c_time);

#endif // SFML_RB_SYSTEM_TIME_H

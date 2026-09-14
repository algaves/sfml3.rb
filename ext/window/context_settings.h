#ifndef SFML_RB_WINDOW_CONTEXT_SETTINGS_H
#define SFML_RB_WINDOW_CONTEXT_SETTINGS_H

#include <ruby.h>

#include "core/sfml.h"

void Init_ContextSettings(VALUE rb_module);

VALUE Get_Klass_ContextSettings(void);

void *Get_ContextSettings_Struct(VALUE self);

sfContextSettings context_settings_from_rb(VALUE rb_settings);

VALUE context_settings_to_rb(sfContextSettings settings);

#endif //SFML_RB_WINDOW_CONTEXT_SETTINGS_H

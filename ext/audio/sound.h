#ifndef SFML_RB_AUDIO_SOUND_H
#define SFML_RB_AUDIO_SOUND_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Sound(VALUE rb_module);

VALUE Get_Klass_Sound(void);

void *Get_Sound_Struct(VALUE self);

#endif //SFML_RB_AUDIO_SOUND_H

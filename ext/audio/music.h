#ifndef SFML_RB_AUDIO_MUSIC_H
#define SFML_RB_AUDIO_MUSIC_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Music(VALUE rb_module);

VALUE Get_Klass_Music(void);

void *Get_Music_Struct(VALUE self);

#endif //SFML_RB_AUDIO_MUSIC_H

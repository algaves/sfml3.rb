#ifndef SFML_RB_AUDIO_SOUND_STREAM_H
#define SFML_RB_AUDIO_SOUND_STREAM_H

#include <ruby.h>

#include "core/sfml.h"

void Init_SoundStream(VALUE rb_module);

VALUE Get_Klass_SoundStream(void);

void *Get_SoundStream_Struct(VALUE self);

#endif //SFML_RB_AUDIO_SOUND_STREAM_H

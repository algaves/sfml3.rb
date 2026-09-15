#ifndef SFML_RB_AUDIO_SOUND_RECORDER_H
#define SFML_RB_AUDIO_SOUND_RECORDER_H

#include <ruby.h>

#include "core/sfml.h"

void Init_SoundRecorder(VALUE rb_module);

VALUE Get_Klass_SoundRecorder(void);

void *Get_SoundRecorder_Struct(VALUE self);

#endif //SFML_RB_AUDIO_SOUND_RECORDER_H

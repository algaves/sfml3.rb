#ifndef SFML_RB_AUDIO_SOUND_BUFFER_RECORDER_H
#define SFML_RB_AUDIO_SOUND_BUFFER_RECORDER_H

#include <ruby.h>

#include "core/sfml.h"

void Init_SoundBufferRecorder(VALUE rb_module);

VALUE Get_Klass_SoundBufferRecorder(void);

void *Get_SoundBufferRecorder_Struct(VALUE self);

#endif //SFML_RB_AUDIO_SOUND_BUFFER_RECORDER_H

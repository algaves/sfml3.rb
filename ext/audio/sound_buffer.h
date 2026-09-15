#ifndef SFML_RB_AUDIO_SOUND_BUFFER_H
#define SFML_RB_AUDIO_SOUND_BUFFER_H

#include <ruby.h>

#include "core/sfml.h"

void Init_SoundBuffer(VALUE rb_module);

VALUE Get_Klass_SoundBuffer(void);

void *Get_SoundBuffer_Struct(VALUE self);

/* Wraps a buffer owned by someone else (a SoundBufferRecorder) without taking
   ownership, so freeing the wrapper must not destroy it. */
VALUE sound_buffer_from_borrowed(const sfSoundBuffer *buffer);

#endif //SFML_RB_AUDIO_SOUND_BUFFER_H

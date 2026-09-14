#ifndef SFML_RB_AUDIO_AUDIO_ENUMS_H
#define SFML_RB_AUDIO_AUDIO_ENUMS_H

#include <ruby.h>

#include "core/sfml.h"

void Init_AudioEnums(VALUE rb_module);

const char *sound_status_name(sfSoundStatus status);

sfSoundStatus sound_status_from_rb(VALUE rb_status);

const char *sound_channel_name(sfSoundChannel channel);

sfSoundChannel sound_channel_from_rb(VALUE rb_channel);

#endif //SFML_RB_AUDIO_AUDIO_ENUMS_H

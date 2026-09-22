#ifndef SFML_RB_AUDIO_SOUND_SOURCE_CONE_H
#define SFML_RB_AUDIO_SOUND_SOURCE_CONE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_SoundSourceCone(VALUE rb_module);

VALUE Get_Klass_SoundSourceCone(void);

void *Get_SoundSourceCone_Struct(VALUE self);

/* Accepts an SF::Audio::SoundSourceCone or a three-element array
   [inner_angle, outer_angle, outer_gain]. */
sfSoundSourceCone sound_source_cone_from_rb(VALUE rb_cone);

VALUE sound_source_cone_to_rb(sfSoundSourceCone cone);

#endif //SFML_RB_AUDIO_SOUND_SOURCE_CONE_H

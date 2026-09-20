#ifndef SFML_RB_AUDIO_SOUND_SOURCE_H
#define SFML_RB_AUDIO_SOUND_SOURCE_H

#include <ruby.h>

/* Every wrapper that mixes in the SoundSource methods (Sound, Music,
   SoundStream) starts with this prefix, so the shared method bodies in
   sound_source.inc can reach the underlying handle and the effect-processor
   slot through RTYPEDDATA_DATA without knowing the concrete struct. */
typedef struct {
    void* handle;
    int effect_slot;
} SoundSource;

void Init_SoundSource(VALUE rb_module);

VALUE Get_Klass_SoundSource(void);

#endif // SFML_RB_AUDIO_SOUND_SOURCE_H

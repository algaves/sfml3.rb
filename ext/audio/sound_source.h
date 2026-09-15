#ifndef SFML_RB_AUDIO_SOUND_SOURCE_H
#define SFML_RB_AUDIO_SOUND_SOURCE_H

/* Every wrapper that mixes in the SoundSource methods (Sound, Music,
   SoundStream) starts with this prefix, so the shared method bodies in
   sound_source.inc can reach the underlying handle and the effect-processor
   slot through RTYPEDDATA_DATA without knowing the concrete struct. */
typedef struct {
    void *handle;
    int effect_slot;
} SoundSource;

#endif //SFML_RB_AUDIO_SOUND_SOURCE_H

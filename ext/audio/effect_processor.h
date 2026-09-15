#ifndef SFML_RB_AUDIO_EFFECT_PROCESSOR_H
#define SFML_RB_AUDIO_EFFECT_PROCESSOR_H

#include <ruby.h>

#include "core/sfml.h"

/* sfEffectProcessor carries no userData, so a single C callback cannot tell
   which sound source invoked it. Instead the binding keeps a fixed pool of
   identical C thunks, each hard-wired to one slot; assigning a processor to a
   source hands it the thunk for that source's slot. Slots are bounded, and
   exceeding the pool raises rather than silently sharing one callback. */
#define EFFECT_PROCESSOR_SLOTS 32

/* Registers every slot as a GC root. Must run before any acquire. */
void Init_EffectProcessor(void);

/* Stores rb_proc in a free slot, writes its index to *slot and returns the
   matching thunk. Passing nil returns NULL (no processor). */
sfEffectProcessor effect_processor_acquire(VALUE rb_proc, int *slot);

/* Frees a slot previously handed out, clearing the Ruby reference. */
void effect_processor_release(int slot);

#endif //SFML_RB_AUDIO_EFFECT_PROCESSOR_H

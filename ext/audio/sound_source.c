#include "audio/sound_source.h"

#include <ruby.h>

static VALUE rb_cSoundSource;

/* Document-class: SFML::SoundSource
 * The common base of every class that plays audio -- SFML::Sound,
 * SFML::SoundStream and SFML::Music. It carries no state of its own; the
 * playback, pitch/pan/volume, spatialization and effect-processor surface is
 * generated per concrete class from ext/audio/sound_source.inc, because each
 * one dispatches to a different CSFML entry point.
 *
 * @!method playing?
 *   Returns +true+ while the source is playing. Rubyesque (Matz-like) over #status.
 *   @return [Boolean]
 * @!method paused?
 *   Returns +true+ while the source is paused. Rubyesque (Matz-like) over #status.
 *   @return [Boolean]
 * @!method stopped?
 *   Returns +true+ when the source is stopped. Rubyesque (Matz-like) over #status.
 *   @return [Boolean]
 */
void Init_SoundSource(VALUE rb_mSFML) {
    rb_cSoundSource = rb_define_class_under(rb_mSFML, "SoundSource", rb_cObject);
}

VALUE Get_Klass_SoundSource(void) {
    return rb_cSoundSource;
}

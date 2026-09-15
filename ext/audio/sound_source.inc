/* Shared method bodies for every sound source (Sound, Music, SoundStream).
   Included from three translation units; each defines SS_FN and SS_METHOD
   before including this file, then undefines them after. Generating concrete
   methods per class keeps the compiler's function-pointer type checks happy,
   which a runtime vtable of cast function pointers would not.

   The wrapper structs all begin with a SoundSource prefix, so the handle and
   the effect-processor slot are reachable without knowing the concrete type. */

#include "audio/sound_source.h"
#include "audio/audio_enums.h"
#include "audio/effect_processor.h"
#include "audio/sound_source_cone.h"
#include "core/macros.h"
#include "system/time.h"
#include "system/vec3.h"

#define SS_HANDLE(self) (((SoundSource *) RTYPEDDATA_DATA(self))->handle)
#define SS_SLOT(self) (((SoundSource *) RTYPEDDATA_DATA(self))->effect_slot)

static VALUE SS_METHOD(play)(VALUE self) {
    SS_FN(play)(SS_HANDLE(self));
    return self;
}

static VALUE SS_METHOD(pause)(VALUE self) {
    SS_FN(pause)(SS_HANDLE(self));
    return self;
}

static VALUE SS_METHOD(stop)(VALUE self) {
    SS_FN(stop)(SS_HANDLE(self));
    return self;
}

static VALUE SS_METHOD(status)(VALUE self) {
    return ID2SYM(rb_intern(sound_status_name(SS_FN(getStatus)(SS_HANDLE(self)))));
}

static VALUE SS_METHOD(looping)(VALUE self) {
    return BOOL2RB(SS_FN(isLooping)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_looping)(VALUE self, VALUE rb_value) {
    SS_FN(setLooping)(SS_HANDLE(self), RTEST(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(pitch)(VALUE self) {
    return DBL2NUM(SS_FN(getPitch)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_pitch)(VALUE self, VALUE rb_value) {
    SS_FN(setPitch)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(pan)(VALUE self) {
    return DBL2NUM(SS_FN(getPan)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_pan)(VALUE self, VALUE rb_value) {
    SS_FN(setPan)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(volume)(VALUE self) {
    return DBL2NUM(SS_FN(getVolume)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_volume)(VALUE self, VALUE rb_value) {
    SS_FN(setVolume)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(spatialization_enabled)(VALUE self) {
    return BOOL2RB(SS_FN(isSpatializationEnabled)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_spatialization_enabled)(VALUE self, VALUE rb_value) {
    SS_FN(setSpatializationEnabled)(SS_HANDLE(self), RTEST(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(position)(VALUE self) {
    return vec3f_to_rb(SS_FN(getPosition)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_position)(VALUE self, VALUE rb_value) {
    SS_FN(setPosition)(SS_HANDLE(self), vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(direction)(VALUE self) {
    return vec3f_to_rb(SS_FN(getDirection)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_direction)(VALUE self, VALUE rb_value) {
    SS_FN(setDirection)(SS_HANDLE(self), vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(velocity)(VALUE self) {
    return vec3f_to_rb(SS_FN(getVelocity)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_velocity)(VALUE self, VALUE rb_value) {
    SS_FN(setVelocity)(SS_HANDLE(self), vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(cone)(VALUE self) {
    return sound_source_cone_to_rb(SS_FN(getCone)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_cone)(VALUE self, VALUE rb_value) {
    SS_FN(setCone)(SS_HANDLE(self), sound_source_cone_from_rb(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(doppler_factor)(VALUE self) {
    return DBL2NUM(SS_FN(getDopplerFactor)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_doppler_factor)(VALUE self, VALUE rb_value) {
    SS_FN(setDopplerFactor)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(directional_attenuation_factor)(VALUE self) {
    return DBL2NUM(SS_FN(getDirectionalAttenuationFactor)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_directional_attenuation_factor)(VALUE self, VALUE rb_value) {
    SS_FN(setDirectionalAttenuationFactor)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(relative_to_listener)(VALUE self) {
    return BOOL2RB(SS_FN(isRelativeToListener)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_relative_to_listener)(VALUE self, VALUE rb_value) {
    SS_FN(setRelativeToListener)(SS_HANDLE(self), RTEST(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(min_distance)(VALUE self) {
    return DBL2NUM(SS_FN(getMinDistance)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_min_distance)(VALUE self, VALUE rb_value) {
    SS_FN(setMinDistance)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(max_distance)(VALUE self) {
    return DBL2NUM(SS_FN(getMaxDistance)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_max_distance)(VALUE self, VALUE rb_value) {
    SS_FN(setMaxDistance)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(min_gain)(VALUE self) {
    return DBL2NUM(SS_FN(getMinGain)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_min_gain)(VALUE self, VALUE rb_value) {
    SS_FN(setMinGain)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(max_gain)(VALUE self) {
    return DBL2NUM(SS_FN(getMaxGain)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_max_gain)(VALUE self, VALUE rb_value) {
    SS_FN(setMaxGain)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(attenuation)(VALUE self) {
    return DBL2NUM(SS_FN(getAttenuation)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_attenuation)(VALUE self, VALUE rb_value) {
    SS_FN(setAttenuation)(SS_HANDLE(self), (float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(playing_offset)(VALUE self) {
    return time_to_rb(SS_FN(getPlayingOffset)(SS_HANDLE(self)));
}

static VALUE SS_METHOD(set_playing_offset)(VALUE self, VALUE rb_value) {
    SS_FN(setPlayingOffset)(SS_HANDLE(self), time_from_rb(rb_value));
    return rb_value;
}

static VALUE SS_METHOD(set_effect_processor)(VALUE self, VALUE rb_proc) {
    sfEffectProcessor processor = effect_processor_acquire(rb_proc, &SS_SLOT(self));

    SS_FN(setEffectProcessor)(SS_HANDLE(self), processor);

    return rb_proc;
}

static void SS_METHOD(define_sound_source_methods)(VALUE klass) {
    rb_define_method(klass, "play", SS_METHOD(play), 0);
    rb_define_method(klass, "pause", SS_METHOD(pause), 0);
    rb_define_method(klass, "stop", SS_METHOD(stop), 0);
    rb_define_method(klass, "status", SS_METHOD(status), 0);
    rb_define_method(klass, "looping?", SS_METHOD(looping), 0);
    rb_define_method(klass, "looping=", SS_METHOD(set_looping), 1);
    rb_define_method(klass, "pitch", SS_METHOD(pitch), 0);
    rb_define_method(klass, "pitch=", SS_METHOD(set_pitch), 1);
    rb_define_method(klass, "pan", SS_METHOD(pan), 0);
    rb_define_method(klass, "pan=", SS_METHOD(set_pan), 1);
    rb_define_method(klass, "volume", SS_METHOD(volume), 0);
    rb_define_method(klass, "volume=", SS_METHOD(set_volume), 1);
    rb_define_method(klass, "spatialization_enabled?", SS_METHOD(spatialization_enabled), 0);
    rb_define_method(klass, "spatialization_enabled=", SS_METHOD(set_spatialization_enabled), 1);
    rb_define_method(klass, "position", SS_METHOD(position), 0);
    rb_define_method(klass, "position=", SS_METHOD(set_position), 1);
    rb_define_method(klass, "direction", SS_METHOD(direction), 0);
    rb_define_method(klass, "direction=", SS_METHOD(set_direction), 1);
    rb_define_method(klass, "velocity", SS_METHOD(velocity), 0);
    rb_define_method(klass, "velocity=", SS_METHOD(set_velocity), 1);
    rb_define_method(klass, "cone", SS_METHOD(cone), 0);
    rb_define_method(klass, "cone=", SS_METHOD(set_cone), 1);
    rb_define_method(klass, "doppler_factor", SS_METHOD(doppler_factor), 0);
    rb_define_method(klass, "doppler_factor=", SS_METHOD(set_doppler_factor), 1);
    rb_define_method(klass, "directional_attenuation_factor",
                     SS_METHOD(directional_attenuation_factor), 0);
    rb_define_method(klass, "directional_attenuation_factor=",
                     SS_METHOD(set_directional_attenuation_factor), 1);
    rb_define_method(klass, "relative_to_listener?", SS_METHOD(relative_to_listener), 0);
    rb_define_method(klass, "relative_to_listener=", SS_METHOD(set_relative_to_listener), 1);
    rb_define_method(klass, "min_distance", SS_METHOD(min_distance), 0);
    rb_define_method(klass, "min_distance=", SS_METHOD(set_min_distance), 1);
    rb_define_method(klass, "max_distance", SS_METHOD(max_distance), 0);
    rb_define_method(klass, "max_distance=", SS_METHOD(set_max_distance), 1);
    rb_define_method(klass, "min_gain", SS_METHOD(min_gain), 0);
    rb_define_method(klass, "min_gain=", SS_METHOD(set_min_gain), 1);
    rb_define_method(klass, "max_gain", SS_METHOD(max_gain), 0);
    rb_define_method(klass, "max_gain=", SS_METHOD(set_max_gain), 1);
    rb_define_method(klass, "attenuation", SS_METHOD(attenuation), 0);
    rb_define_method(klass, "attenuation=", SS_METHOD(set_attenuation), 1);
    rb_define_method(klass, "playing_offset", SS_METHOD(playing_offset), 0);
    rb_define_method(klass, "playing_offset=", SS_METHOD(set_playing_offset), 1);
    rb_define_method(klass, "effect_processor=", SS_METHOD(set_effect_processor), 1);
}

#undef SS_HANDLE
#undef SS_SLOT

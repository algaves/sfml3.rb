#include "audio/audio_enums.h"

#include <ruby.h>
#include <string.h>

static const char* sound_status_names[] = {"stopped", "paused", "playing"};

/* Designated initializers, indexed by the enum constant rather than by
   position: CSFML 3 defines the channel order, and a positional table would
   silently return the wrong name if a future release inserted a value. */
static const char* sound_channel_names[] = {
    [sfSoundChannelUnspecified] = "unspecified",
    [sfSoundChannelMono] = "mono",
    [sfSoundChannelFrontLeft] = "front_left",
    [sfSoundChannelFrontRight] = "front_right",
    [sfSoundChannelFrontCenter] = "front_center",
    [sfSoundChannelFrontLeftOfCenter] = "front_left_of_center",
    [sfSoundChannelFrontRightOfCenter] = "front_right_of_center",
    [sfSoundChannelLowFrequencyEffects] = "low_frequency_effects",
    [sfSoundChannelBackLeft] = "back_left",
    [sfSoundChannelBackRight] = "back_right",
    [sfSoundChannelBackCenter] = "back_center",
    [sfSoundChannelSideLeft] = "side_left",
    [sfSoundChannelSideRight] = "side_right",
    [sfSoundChannelTopCenter] = "top_center",
    [sfSoundChannelTopFrontLeft] = "top_front_left",
    [sfSoundChannelTopFrontRight] = "top_front_right",
    [sfSoundChannelTopFrontCenter] = "top_front_center",
    [sfSoundChannelTopBackLeft] = "top_back_left",
    [sfSoundChannelTopBackRight] = "top_back_right",
    [sfSoundChannelTopBackCenter] = "top_back_center"};

static int symbol_index(VALUE rb_symbol, const char* const* names, size_t count) {
    const char* name;
    size_t i;

    if (!SYMBOL_P(rb_symbol)) {
        return -1;
    }

    name = rb_id2name(SYM2ID(rb_symbol));

    for (i = 0; i < count; i++) {
        if (names[i] != NULL && strcmp(name, names[i]) == 0) {
            return (int)i;
        }
    }

    return -1;
}

const char* sound_status_name(sfSoundStatus status) {
    if (status >= sfStopped && status <= sfPlaying) {
        return sound_status_names[status];
    }

    return "stopped";
}

sfSoundStatus sound_status_from_rb(VALUE rb_status) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_status)) {
        return (sfSoundStatus)NUM2INT(rb_status);
    }

    index = symbol_index(rb_status, sound_status_names, 3);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown sound status");
    }

    return (sfSoundStatus)index;
}

const char* sound_channel_name(sfSoundChannel channel) {
    if (channel >= sfSoundChannelUnspecified && channel <= sfSoundChannelTopBackCenter &&
        sound_channel_names[channel] != NULL) {
        return sound_channel_names[channel];
    }

    return "unspecified";
}

sfSoundChannel sound_channel_from_rb(VALUE rb_channel) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_channel)) {
        return (sfSoundChannel)NUM2INT(rb_channel);
    }

    index = symbol_index(rb_channel, sound_channel_names,
                         sizeof(sound_channel_names) / sizeof(sound_channel_names[0]));

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown sound channel");
    }

    return (sfSoundChannel)index;
}

/* Document-module: SF::Audio::SoundStatus
 * Playback state constants returned by SoundSource#status (Sound, Music,
 * SoundStream). Mirrored as symbols (+:stopped+, +:paused+, +:playing+) by
 * that method rather than these Integer constants, but both refer to the
 * same underlying values.
 */

/* Document-module: SF::Audio::SoundChannel
 * Speaker position constants used in a channel map (SoundBuffer#channel_map,
 * Music#channel_map, SoundStream#channel_map, SoundRecorder#channel_map),
 * mirrored as symbols (e.g. +:front_left+) rather than these Integer
 * constants in those methods.
 */
void Init_AudioEnums(VALUE rb_mAudio) {
    VALUE rb_mSoundStatus = rb_define_module_under(rb_mAudio, "SoundStatus");
    VALUE rb_mSoundChannel = rb_define_module_under(rb_mAudio, "SoundChannel");

    /* The source is not playing. */
    rb_define_const(rb_mSoundStatus, "STOPPED", INT2NUM(sfStopped));
    /* The source is paused; #playing_offset stays where it was paused. */
    rb_define_const(rb_mSoundStatus, "PAUSED", INT2NUM(sfPaused));
    /* The source is currently playing. */
    rb_define_const(rb_mSoundStatus, "PLAYING", INT2NUM(sfPlaying));

    /* No channel position specified. */
    rb_define_const(rb_mSoundChannel, "UNSPECIFIED", INT2NUM(sfSoundChannelUnspecified));
    /* A single, centered channel. */
    rb_define_const(rb_mSoundChannel, "MONO", INT2NUM(sfSoundChannelMono));
    /* Front left speaker. */
    rb_define_const(rb_mSoundChannel, "FRONT_LEFT", INT2NUM(sfSoundChannelFrontLeft));
    /* Front right speaker. */
    rb_define_const(rb_mSoundChannel, "FRONT_RIGHT", INT2NUM(sfSoundChannelFrontRight));
    /* Front center speaker. */
    rb_define_const(rb_mSoundChannel, "FRONT_CENTER", INT2NUM(sfSoundChannelFrontCenter));
    /* Front left-of-center speaker. */
    rb_define_const(rb_mSoundChannel, "FRONT_LEFT_OF_CENTER",
                    INT2NUM(sfSoundChannelFrontLeftOfCenter));
    /* Front right-of-center speaker. */
    rb_define_const(rb_mSoundChannel, "FRONT_RIGHT_OF_CENTER",
                    INT2NUM(sfSoundChannelFrontRightOfCenter));
    /* Low-frequency effects (subwoofer) channel. */
    rb_define_const(rb_mSoundChannel, "LOW_FREQUENCY_EFFECTS",
                    INT2NUM(sfSoundChannelLowFrequencyEffects));
    /* Back left speaker. */
    rb_define_const(rb_mSoundChannel, "BACK_LEFT", INT2NUM(sfSoundChannelBackLeft));
    /* Back right speaker. */
    rb_define_const(rb_mSoundChannel, "BACK_RIGHT", INT2NUM(sfSoundChannelBackRight));
    /* Back center speaker. */
    rb_define_const(rb_mSoundChannel, "BACK_CENTER", INT2NUM(sfSoundChannelBackCenter));
    /* Side left speaker. */
    rb_define_const(rb_mSoundChannel, "SIDE_LEFT", INT2NUM(sfSoundChannelSideLeft));
    /* Side right speaker. */
    rb_define_const(rb_mSoundChannel, "SIDE_RIGHT", INT2NUM(sfSoundChannelSideRight));
    /* Top center speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_CENTER", INT2NUM(sfSoundChannelTopCenter));
    /* Top front left speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_FRONT_LEFT", INT2NUM(sfSoundChannelTopFrontLeft));
    /* Top front right speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_FRONT_RIGHT", INT2NUM(sfSoundChannelTopFrontRight));
    /* Top front center speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_FRONT_CENTER", INT2NUM(sfSoundChannelTopFrontCenter));
    /* Top back left speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_BACK_LEFT", INT2NUM(sfSoundChannelTopBackLeft));
    /* Top back right speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_BACK_RIGHT", INT2NUM(sfSoundChannelTopBackRight));
    /* Top back center speaker. */
    rb_define_const(rb_mSoundChannel, "TOP_BACK_CENTER", INT2NUM(sfSoundChannelTopBackCenter));
}

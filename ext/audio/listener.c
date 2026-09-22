#include "audio/listener.h"

#include <ruby.h>

#include "audio/sound_source_cone.h"
#include "system/vec3.h"

/* call-seq: global_volume -> Float
 *
 * Returns the global volume applied to every sound source in the scene.
 *
 * @return [Float] 0 to 100
 */
static VALUE Listener_global_volume(VALUE module) {
    return DBL2NUM(sfListener_getGlobalVolume());
}

/* call-seq:
 *   global_volume=(value) -> Float
 *
 * Sets the global volume all sound sources are scaled by.
 *
 * @return [Float] +value+
 */
static VALUE Listener_set_global_volume(VALUE module, VALUE rb_value) {
    sfListener_setGlobalVolume((float)NUM2DBL(rb_value));
    return rb_value;
}

/* call-seq: position -> Vector3
 *
 * Returns the listener's position in the 3D audio scene.
 *
 * @return [Vector3]
 */
static VALUE Listener_position(VALUE module) {
    return vec3f_to_rb(sfListener_getPosition());
}

/* call-seq:
 *   position=(value) -> Vector3
 *
 * Sets the listener's position in the 3D audio scene.
 *
 * @return [Vector3] +value+
 */
static VALUE Listener_set_position(VALUE module, VALUE rb_value) {
    sfListener_setPosition(vec3f_from_rb(rb_value));
    return rb_value;
}

/* call-seq: direction -> Vector3
 *
 * Returns the direction the listener is facing, used for spatialization.
 *
 * @return [Vector3]
 */
static VALUE Listener_direction(VALUE module) {
    return vec3f_to_rb(sfListener_getDirection());
}

/* call-seq:
 *   direction=(value) -> Vector3
 *
 * Sets the direction the listener is facing.
 *
 * @return [Vector3] +value+
 */
static VALUE Listener_set_direction(VALUE module, VALUE rb_value) {
    sfListener_setDirection(vec3f_from_rb(rb_value));
    return rb_value;
}

/* call-seq: velocity -> Vector3
 *
 * Returns the listener's velocity, used for Doppler calculations.
 *
 * @return [Vector3]
 */
static VALUE Listener_velocity(VALUE module) {
    return vec3f_to_rb(sfListener_getVelocity());
}

/* call-seq:
 *   velocity=(value) -> Vector3
 *
 * Sets the listener's velocity for Doppler calculations.
 *
 * @return [Vector3] +value+
 */
static VALUE Listener_set_velocity(VALUE module, VALUE rb_value) {
    sfListener_setVelocity(vec3f_from_rb(rb_value));
    return rb_value;
}

/* call-seq: up_vector -> Vector3
 *
 * Returns the listener's up vector, defining its vertical orientation.
 *
 * @return [Vector3]
 */
static VALUE Listener_up_vector(VALUE module) {
    return vec3f_to_rb(sfListener_getUpVector());
}

/* call-seq:
 *   up_vector=(value) -> Vector3
 *
 * Sets the listener's up vector, defining its vertical orientation.
 *
 * @return [Vector3] +value+
 */
static VALUE Listener_set_up_vector(VALUE module, VALUE rb_value) {
    sfListener_setUpVector(vec3f_from_rb(rb_value));
    return rb_value;
}

/* call-seq: cone -> SoundSourceCone
 *
 * Returns the listener's directional attenuation cone.
 *
 * @return [SoundSourceCone]
 */
static VALUE Listener_cone(VALUE module) {
    sfListenerCone cone = sfListener_getCone();

    return sound_source_cone_to_rb(
        (sfSoundSourceCone){cone.innerAngle, cone.outerAngle, cone.outerGain});
}

/* call-seq:
 *   cone=(value) -> SoundSourceCone
 *
 * Sets the listener's directional attenuation cone.
 *
 * @return [SoundSourceCone] +value+
 */
static VALUE Listener_set_cone(VALUE module, VALUE rb_value) {
    sfSoundSourceCone cone = sound_source_cone_from_rb(rb_value);

    sfListener_setCone((sfListenerCone){cone.innerAngle, cone.outerAngle, cone.outerGain});

    return rb_value;
}

/* Document-module: SF::Audio::Listener
 * Module functions controlling the single global audio listener: its
 * position, orientation and volume, against which every SoundSource's
 * spatialization is computed.
 */
void Init_Listener(VALUE rb_mAudio) {
    VALUE rb_mListener = rb_define_module_under(rb_mAudio, "Listener");

    rb_define_module_function(rb_mListener, "global_volume", Listener_global_volume, 0);
    rb_define_module_function(rb_mListener, "global_volume=", Listener_set_global_volume, 1);
    rb_define_module_function(rb_mListener, "position", Listener_position, 0);
    rb_define_module_function(rb_mListener, "position=", Listener_set_position, 1);
    rb_define_module_function(rb_mListener, "direction", Listener_direction, 0);
    rb_define_module_function(rb_mListener, "direction=", Listener_set_direction, 1);
    rb_define_module_function(rb_mListener, "velocity", Listener_velocity, 0);
    rb_define_module_function(rb_mListener, "velocity=", Listener_set_velocity, 1);
    rb_define_module_function(rb_mListener, "up_vector", Listener_up_vector, 0);
    rb_define_module_function(rb_mListener, "up_vector=", Listener_set_up_vector, 1);
    rb_define_module_function(rb_mListener, "cone", Listener_cone, 0);
    rb_define_module_function(rb_mListener, "cone=", Listener_set_cone, 1);
}

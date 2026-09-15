#include "audio/listener.h"

#include <ruby.h>

#include "audio/sound_source_cone.h"
#include "system/vec3.h"

static VALUE Listener_global_volume(VALUE module) {
    return DBL2NUM(sfListener_getGlobalVolume());
}

static VALUE Listener_set_global_volume(VALUE module, VALUE rb_value) {
    sfListener_setGlobalVolume((float) NUM2DBL(rb_value));
    return rb_value;
}

static VALUE Listener_position(VALUE module) {
    return vec3f_to_rb(sfListener_getPosition());
}

static VALUE Listener_set_position(VALUE module, VALUE rb_value) {
    sfListener_setPosition(vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE Listener_direction(VALUE module) {
    return vec3f_to_rb(sfListener_getDirection());
}

static VALUE Listener_set_direction(VALUE module, VALUE rb_value) {
    sfListener_setDirection(vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE Listener_velocity(VALUE module) {
    return vec3f_to_rb(sfListener_getVelocity());
}

static VALUE Listener_set_velocity(VALUE module, VALUE rb_value) {
    sfListener_setVelocity(vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE Listener_up_vector(VALUE module) {
    return vec3f_to_rb(sfListener_getUpVector());
}

static VALUE Listener_set_up_vector(VALUE module, VALUE rb_value) {
    sfListener_setUpVector(vec3f_from_rb(rb_value));
    return rb_value;
}

static VALUE Listener_cone(VALUE module) {
    sfListenerCone cone = sfListener_getCone();

    return sound_source_cone_to_rb((sfSoundSourceCone) {cone.innerAngle, cone.outerAngle,
                                                        cone.outerGain});
}

static VALUE Listener_set_cone(VALUE module, VALUE rb_value) {
    sfSoundSourceCone cone = sound_source_cone_from_rb(rb_value);

    sfListener_setCone((sfListenerCone) {cone.innerAngle, cone.outerAngle, cone.outerGain});

    return rb_value;
}

void Init_Listener(VALUE rb_module) {
    VALUE rb_mListener = rb_define_module_under(rb_module, "Listener");

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

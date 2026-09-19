#include "audio/sound_source_cone.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfSoundSourceCone cone;
} SoundSourceCone;

static VALUE rb_cSoundSourceCone;

static void SoundSourceCone_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t SoundSourceCone_data_type = {
    .wrap_struct_name = "SFML::SoundSourceCone",
    .function = {.dmark = NULL, .dfree = SoundSourceCone_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE SoundSourceCone_wrap(sfSoundSourceCone cone) {
    SoundSourceCone* ptr = malloc(sizeof(SoundSourceCone));

    ptr->cone = cone;

    return TypedData_Wrap_Struct(rb_cSoundSourceCone, &SoundSourceCone_data_type, ptr);
}

static VALUE SoundSourceCone_alloc(VALUE klass) {
    SoundSourceCone* ptr = malloc(sizeof(SoundSourceCone));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate sound source cone");
    }

    ptr->cone = (sfSoundSourceCone){0.0f, 0.0f, 0.0f};

    return TypedData_Wrap_Struct(klass, &SoundSourceCone_data_type, ptr);
}

/* call-seq:
 *   SoundSourceCone.new(inner_angle, outer_angle, outer_gain) -> SoundSourceCone
 *
 * Creates a directional attenuation cone with the given angles and gain.
 *
 * @return [SoundSourceCone]
 */
static VALUE SoundSourceCone_initialize(VALUE self, VALUE rb_inner, VALUE rb_outer, VALUE rb_gain) {
    sfSoundSourceCone cone = {.innerAngle = (float)NUM2DBL(rb_inner),
                              .outerAngle = (float)NUM2DBL(rb_outer),
                              .outerGain = (float)NUM2DBL(rb_gain)};

    ((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone = cone;

    return self;
}

/* call-seq: inner_angle -> Float
 *
 * Returns the inner cone angle in degrees.
 *
 * @return [Float] angle, in degrees, of the inner cone within which the
 *   source is heard at full volume
 */
static VALUE SoundSourceCone_get_inner_angle(VALUE self) {
    return DBL2NUM(((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone.innerAngle);
}

/* call-seq: outer_angle -> Float
 *
 * Returns the outer cone angle in degrees.
 *
 * @return [Float] angle, in degrees, of the outer cone beyond which the
 *   source is heard at +outer_gain+ volume
 */
static VALUE SoundSourceCone_get_outer_angle(VALUE self) {
    return DBL2NUM(((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone.outerAngle);
}

/* call-seq: outer_gain -> Float
 *
 * Returns the gain applied outside the outer cone.
 *
 * @return [Float] volume factor applied outside the outer cone
 */
static VALUE SoundSourceCone_get_outer_gain(VALUE self) {
    return DBL2NUM(((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone.outerGain);
}

/* call-seq:
 *   inner_angle=(value) -> Float
 *
 * Sets the inner cone angle in degrees.
 *
 * @return [Float] +value+
 */
static VALUE SoundSourceCone_set_inner_angle(VALUE self, VALUE rb_value) {
    ((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone.innerAngle =
        (float)NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq:
 *   outer_angle=(value) -> Float
 *
 * Sets the outer cone angle in degrees.
 *
 * @return [Float] +value+
 */
static VALUE SoundSourceCone_set_outer_angle(VALUE self, VALUE rb_value) {
    ((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone.outerAngle =
        (float)NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq:
 *   outer_gain=(value) -> Float
 *
 * Sets the gain applied outside the outer cone.
 *
 * @return [Float] +value+
 */
static VALUE SoundSourceCone_set_outer_gain(VALUE self, VALUE rb_value) {
    ((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone.outerGain = (float)NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq: to_a -> [Float, Float, Float]
 *
 * Returns the cone's three components as an Array.
 *
 * @return [Array<Float>] +[inner_angle, outer_angle, outer_gain]+
 */
static VALUE SoundSourceCone_to_a(VALUE self) {
    sfSoundSourceCone cone = ((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone;

    return rb_ary_new_from_args(3, DBL2NUM(cone.innerAngle), DBL2NUM(cone.outerAngle),
                                DBL2NUM(cone.outerGain));
}

/* call-seq:
 *   self == other -> true or false
 *
 * Returns +true+ if +other+ is a cone with the same angles and gain.
 *
 * @return [Boolean]
 */
static VALUE SoundSourceCone_eql(VALUE self, VALUE rb_other) {
    sfSoundSourceCone a = ((SoundSourceCone*)Get_SoundSourceCone_Struct(self))->cone;
    sfSoundSourceCone b;

    if (!rb_obj_is_kind_of(rb_other, rb_cSoundSourceCone)) {
        return Qfalse;
    }

    b = ((SoundSourceCone*)Get_SoundSourceCone_Struct(rb_other))->cone;

    return BOOL2RB(a.innerAngle == b.innerAngle && a.outerAngle == b.outerAngle &&
                   a.outerGain == b.outerGain);
}

/* Document-class: SFML::SoundSourceCone
 * A directional attenuation cone for a SoundSource (SoundSource#cone) or the
 * Listener (Listener.cone): within +inner_angle+ degrees of the facing
 * direction the source is heard at full volume; beyond +outer_angle+
 * degrees it is heard at +outer_gain+ volume, with a smooth transition in
 * between.
 *
 * @!attribute inner_angle
 *   Angle, in degrees, of the inner cone within which the source is heard at
 *   full volume.
 *   @return [Float] angle, in degrees, of the inner cone within which the
 *     source is heard at full volume
 * @!attribute outer_angle
 *   Angle, in degrees, of the outer cone beyond which the source is heard at
 *   +outer_gain+ volume.
 *   @return [Float] angle, in degrees, of the outer cone beyond which the
 *     source is heard at +outer_gain+ volume
 * @!attribute outer_gain
 *   Volume factor applied outside the outer cone.
 *   @return [Float] volume factor applied outside the outer cone
 */
void Init_SoundSourceCone(VALUE rb_mSFML) {
    rb_cSoundSourceCone = rb_define_class_under(rb_mSFML, "SoundSourceCone", rb_cObject);

    rb_define_alloc_func(rb_cSoundSourceCone, SoundSourceCone_alloc);
    rb_define_method(rb_cSoundSourceCone, "initialize", SoundSourceCone_initialize, 3);

    rb_define_method(rb_cSoundSourceCone, "inner_angle", SoundSourceCone_get_inner_angle, 0);
    rb_define_method(rb_cSoundSourceCone, "outer_angle", SoundSourceCone_get_outer_angle, 0);
    rb_define_method(rb_cSoundSourceCone, "outer_gain", SoundSourceCone_get_outer_gain, 0);
    rb_define_method(rb_cSoundSourceCone, "inner_angle=", SoundSourceCone_set_inner_angle, 1);
    rb_define_method(rb_cSoundSourceCone, "outer_angle=", SoundSourceCone_set_outer_angle, 1);
    rb_define_method(rb_cSoundSourceCone, "outer_gain=", SoundSourceCone_set_outer_gain, 1);
    rb_define_method(rb_cSoundSourceCone, "to_a", SoundSourceCone_to_a, 0);
    rb_define_method(rb_cSoundSourceCone, "==", SoundSourceCone_eql, 1);
}

VALUE Get_Klass_SoundSourceCone(void) {
    return rb_cSoundSourceCone;
}

void* Get_SoundSourceCone_Struct(VALUE self) {
    SoundSourceCone* ptr;
    TypedData_Get_Struct(self, SoundSourceCone, &SoundSourceCone_data_type, ptr);
    return ptr;
}

sfSoundSourceCone sound_source_cone_from_rb(VALUE rb_cone) {
    if (rb_obj_is_kind_of(rb_cone, rb_cSoundSourceCone)) {
        return ((SoundSourceCone*)Get_SoundSourceCone_Struct(rb_cone))->cone;
    }

    if (RB_TYPE_P(rb_cone, T_ARRAY) && RARRAY_LEN(rb_cone) == 3) {
        sfSoundSourceCone cone = {.innerAngle = (float)NUM2DBL(rb_ary_entry(rb_cone, 0)),
                                  .outerAngle = (float)NUM2DBL(rb_ary_entry(rb_cone, 1)),
                                  .outerGain = (float)NUM2DBL(rb_ary_entry(rb_cone, 2))};

        return cone;
    }

    raise_invalid_argument_class(rb_cSoundSourceCone);

    return (sfSoundSourceCone){0.0f, 0.0f, 0.0f};
}

VALUE sound_source_cone_to_rb(sfSoundSourceCone cone) {
    return SoundSourceCone_wrap(cone);
}

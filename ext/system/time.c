#include "system/time.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfTime time;
} Time;

static VALUE rb_cSFTime;

static void Time_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Time_data_type = {
    .wrap_struct_name = "SFML::Time",
    .function = {.dmark = NULL, .dfree = Time_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Time_wrap(sfTime time) {
    Time* ptr = malloc(sizeof(Time));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate time");
    }

    ptr->time = time;

    return TypedData_Wrap_Struct(rb_cSFTime, &Time_data_type, ptr);
}

/* call-seq:
 *   Time.new             -> Time(0)
 *   Time.new(seconds)    -> Time
 *   Time.new(microseconds_integer) -> Time
 *   Time.new(other_time) -> copy of +other_time+
 *
 * A Float argument is interpreted as seconds; an Integer argument is
 * interpreted as microseconds. Prefer .seconds/.milliseconds/.microseconds
 * to be explicit.
 *
 * @return [Time]
 * @raise [ArgumentError] if given more than one argument
 */
static VALUE Time_new(int argc, VALUE* argv, VALUE klass) {
    VALUE self;
    Time* ptr;
    sfTime time = sfTime_Zero;

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cSFTime)) {
        time = ((Time*)Get_Time_Struct(argv[0]))->time;
    } else if (argc == 1 && RB_INTEGER_TYPE_P(argv[0])) {
        time = sfMicroseconds((int64_t)NUM2LL(argv[0]));
    } else if (argc == 1) {
        time = sfSeconds((float)NUM2DBL(argv[0]));
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(1, argc);
    }

    ptr = malloc(sizeof(Time));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate time");
    }

    ptr->time = time;

    self = TypedData_Wrap_Struct(klass, &Time_data_type, ptr);

    return self;
}

/* call-seq:
 *   Time.seconds(value) -> Time
 *
 * Creates a Time of +value+ seconds, where +value+ is any Float-convertible
 * number.
 *
 * @return [Time]
 */
static VALUE Time_seconds(VALUE klass, VALUE rb_seconds) {
    return Time_wrap(sfSeconds((float)NUM2DBL(rb_seconds)));
}

/* call-seq:
 *   Time.milliseconds(value) -> Time
 *
 * Creates a Time of +value+ milliseconds.
 *
 * @return [Time]
 */
static VALUE Time_milliseconds(VALUE klass, VALUE rb_milliseconds) {
    return Time_wrap(sfMilliseconds((int32_t)NUM2INT(rb_milliseconds)));
}

/* call-seq:
 *   Time.microseconds(value) -> Time
 *
 * Creates a Time of +value+ microseconds.
 *
 * @return [Time]
 */
static VALUE Time_microseconds(VALUE klass, VALUE rb_microseconds) {
    return Time_wrap(sfMicroseconds((int64_t)NUM2LL(rb_microseconds)));
}

/* call-seq:
 *   Time.zero -> Time
 *
 * Returns the zero-duration Time.
 *
 * @return [Time] a zero-duration Time
 */
static VALUE Time_zero(VALUE klass) {
    return Time_wrap(sfTime_Zero);
}

/* call-seq: as_seconds -> Float
 *
 * Returns the duration expressed in seconds.
 *
 * @return [Float]
 */
static VALUE Time_as_seconds(VALUE self) {
    return DBL2NUM(sfTime_asSeconds(((Time*)Get_Time_Struct(self))->time));
}

/* call-seq: as_milliseconds -> Integer
 *
 * Returns the duration expressed in whole milliseconds.
 *
 * @return [Integer]
 */
static VALUE Time_as_milliseconds(VALUE self) {
    return INT2NUM(sfTime_asMilliseconds(((Time*)Get_Time_Struct(self))->time));
}

/* call-seq: as_microseconds -> Integer
 *
 * Returns the duration expressed in whole microseconds.
 *
 * @return [Integer]
 */
static VALUE Time_as_microseconds(VALUE self) {
    return LL2NUM(sfTime_asMicroseconds(((Time*)Get_Time_Struct(self))->time));
}

/* call-seq: to_f -> Float
 *
 * Alias for #as_seconds.
 *
 * @return [Float]
 */
static VALUE Time_to_f(VALUE self) {
    return Time_as_seconds(self);
}

/* call-seq: to_i -> Integer
 *
 * Alias for #as_microseconds.
 *
 * @return [Integer]
 */
static VALUE Time_to_i(VALUE self) {
    return Time_as_microseconds(self);
}

/* call-seq:
 *   self + other -> Time
 *
 * Adds +other+ to the duration, returning a new Time.
 *
 * @return [Time]
 */
static VALUE Time_add(VALUE self, VALUE rb_other) {
    sfTime a = ((Time*)Get_Time_Struct(self))->time;
    sfTime b = time_from_rb(rb_other);

    return Time_wrap(sfMicroseconds(sfTime_asMicroseconds(a) + sfTime_asMicroseconds(b)));
}

/* call-seq:
 *   self - other -> Time
 *
 * Subtracts +other+ from the duration, returning a new Time.
 *
 * @return [Time]
 */
static VALUE Time_sub(VALUE self, VALUE rb_other) {
    sfTime a = ((Time*)Get_Time_Struct(self))->time;
    sfTime b = time_from_rb(rb_other);

    return Time_wrap(sfMicroseconds(sfTime_asMicroseconds(a) - sfTime_asMicroseconds(b)));
}

/* call-seq:
 *   self * scalar -> Time
 *
 * Scales the duration by +scalar+, returning a new Time.
 *
 * @return [Time]
 */
static VALUE Time_mul(VALUE self, VALUE rb_scalar) {
    sfTime a = ((Time*)Get_Time_Struct(self))->time;

    return Time_wrap(sfMicroseconds((int64_t)(sfTime_asMicroseconds(a) * NUM2DBL(rb_scalar))));
}

/* call-seq:
 *   self / other -> Time or Float
 *
 * Dividing by a Time returns the (unitless) ratio of the two durations as a
 * Float; dividing by a Number scales the duration.
 *
 * @return [Time, Float]
 * @raise [ZeroDivError] if dividing by a zero Time
 */
static VALUE Time_div(VALUE self, VALUE rb_other) {
    sfTime a = ((Time*)Get_Time_Struct(self))->time;

    if (rb_obj_is_kind_of(rb_other, rb_cSFTime)) {
        int64_t b = sfTime_asMicroseconds(((Time*)Get_Time_Struct(rb_other))->time);

        if (b == 0) {
            rb_raise(rb_eZeroDivError, "divided by 0");
        }

        return DBL2NUM((double)sfTime_asMicroseconds(a) / (double)b);
    }

    return Time_wrap(sfMicroseconds((int64_t)(sfTime_asMicroseconds(a) / NUM2DBL(rb_other))));
}

/* call-seq:
 *   self <=> other -> -1, 0 or 1
 *
 * Compares the duration with +other+, which may be a Time or a number.
 *
 * @return [Integer]
 */
static VALUE Time_cmp(VALUE self, VALUE rb_other) {
    sfTime a = ((Time*)Get_Time_Struct(self))->time;
    sfTime b = time_from_rb(rb_other);
    int64_t micro_a = sfTime_asMicroseconds(a);
    int64_t micro_b = sfTime_asMicroseconds(b);

    if (micro_a < micro_b) {
        return INT2NUM(-1);
    }

    if (micro_a > micro_b) {
        return INT2NUM(1);
    }

    return INT2NUM(0);
}

/* call-seq:
 *   self == other -> true or false
 *
 * Returns +true+ if +other+ is a Time holding the same duration.
 *
 * @return [Boolean]
 */
static VALUE Time_eql(VALUE self, VALUE rb_other) {
    if (!rb_obj_is_kind_of(rb_other, rb_cSFTime)) {
        return Qfalse;
    }

    return BOOL2RB(sfTime_asMicroseconds(((Time*)Get_Time_Struct(self))->time) ==
                   sfTime_asMicroseconds(((Time*)Get_Time_Struct(rb_other))->time));
}

/* call-seq: to_s -> String
 *
 * Returns the duration formatted as a number of seconds.
 *
 * @return [String] +"N s"+, e.g. +"1.5 s"+
 */
static VALUE Time_to_s(VALUE self) {
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "%g s",
             sfTime_asSeconds(((Time*)Get_Time_Struct(self))->time));

    return rb_str_new2(buffer);
}

/* Document-class: SFML::Time
 * A time duration, with microsecond precision internally. Used throughout
 * the library wherever a duration or timestamp is needed (Clock#elapsed_time,
 * SoundSource#playing_offset, sleep, animation timers, ...).
 *
 * Includes +Comparable+.
 */
void Init_Time(VALUE rb_mSFML) {
    rb_cSFTime = rb_define_class_under(rb_mSFML, "Time", rb_cObject);

    rb_include_module(rb_cSFTime, rb_mComparable);

    rb_define_singleton_method(rb_cSFTime, "new", Time_new, -1);
    rb_define_singleton_method(rb_cSFTime, "seconds", Time_seconds, 1);
    rb_define_singleton_method(rb_cSFTime, "milliseconds", Time_milliseconds, 1);
    rb_define_singleton_method(rb_cSFTime, "microseconds", Time_microseconds, 1);
    rb_define_singleton_method(rb_cSFTime, "zero", Time_zero, 0);

    rb_define_method(rb_cSFTime, "as_seconds", Time_as_seconds, 0);
    rb_define_method(rb_cSFTime, "as_milliseconds", Time_as_milliseconds, 0);
    rb_define_method(rb_cSFTime, "as_microseconds", Time_as_microseconds, 0);
    rb_define_method(rb_cSFTime, "to_f", Time_to_f, 0);
    rb_define_method(rb_cSFTime, "to_i", Time_to_i, 0);

    rb_define_method(rb_cSFTime, "+", Time_add, 1);
    rb_define_method(rb_cSFTime, "-", Time_sub, 1);
    rb_define_method(rb_cSFTime, "*", Time_mul, 1);
    rb_define_method(rb_cSFTime, "/", Time_div, 1);
    rb_define_method(rb_cSFTime, "<=>", Time_cmp, 1);
    rb_define_method(rb_cSFTime, "==", Time_eql, 1);
    rb_define_method(rb_cSFTime, "to_s", Time_to_s, 0);
}

VALUE Get_Klass_Time(void) {
    return rb_cSFTime;
}

void* Get_Time_Struct(VALUE self) {
    Time* ptr;
    TypedData_Get_Struct(self, Time, &Time_data_type, ptr);
    return ptr;
}

sfTime time_from_rb(VALUE rb_time) {
    if (rb_obj_is_kind_of(rb_time, rb_cSFTime)) {
        return ((Time*)Get_Time_Struct(rb_time))->time;
    }

    if (RB_INTEGER_TYPE_P(rb_time)) {
        return sfMicroseconds((int64_t)NUM2LL(rb_time));
    }

    if (RB_FLOAT_TYPE_P(rb_time) || RB_TYPE_P(rb_time, T_RATIONAL)) {
        return sfSeconds((float)NUM2DBL(rb_time));
    }

    raise_invalid_argument_class(rb_cSFTime);

    return sfTime_Zero;
}

VALUE time_to_rb(sfTime c_time) {
    return Time_wrap(c_time);
}

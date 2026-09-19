#include "system/sleep.h"

#include <ruby.h>

#include "system/time.h"
#include "core/sfml.h"

/* call-seq:
 *   SFML.sleep(duration) -> duration
 *
 * Blocks the calling thread for +duration+, which may be a Time, a number
 * of seconds, or anything #to_f-convertible.
 *
 * @return [Time, Numeric] +duration+
 */
static VALUE Sleep_sleep(VALUE module, VALUE rb_duration) {
    sfSleep(time_from_rb(rb_duration));

    return rb_duration;
}

void Init_Sleep(VALUE rb_mSFML) {
    rb_define_module_function(rb_mSFML, "sleep", Sleep_sleep, 1);
}

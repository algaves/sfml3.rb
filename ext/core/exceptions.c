#include "core/exceptions.h"

#include <string.h>

#define MSG_LENGTH 86

void raise_invalid_argument_type(const char* type) {
    char msg[MSG_LENGTH];

    snprintf(msg, MSG_LENGTH, "invalid argument, expected a %s object", type);

    rb_raise(rb_eArgError, "%s", msg);
}

void raise_invalid_argument_class(VALUE rb_cKlass) {
    VALUE rb_name = rb_funcall(rb_cKlass, rb_intern("name"), 0);

    raise_invalid_argument_type(NIL_P(rb_name) ? "anonymous class" : RSTRING_PTR(rb_name));
}

void raise_invalid_arguments_excepted(int expected, size_t given) {
    char msg[MSG_LENGTH];

    if (expected > 0) {
        snprintf(msg, MSG_LENGTH, "wrong number of arguments (given %zu, expected %i)", given,
                 expected);
    } else {
        snprintf(msg, MSG_LENGTH, "wrong number of arguments (given %zu)", given);
    }

    rb_raise(rb_eArgError, "%s", msg);
}

void raise_invalid_array_length(size_t length) {
    char msg[MSG_LENGTH];

    snprintf(msg, MSG_LENGTH, "invalid array length, expected length of %zu", length);

    rb_raise(rb_eArgError, "%s", msg);
}

void raise_method_no_implemented(const char* method) {
    if (method != NULL && strlen(method) > 0) {
        rb_raise(rb_eNotImpError, "method not implemented: %s", method);
    } else {
        rb_raise(rb_eNotImpError, "method not implemented");
    }
}
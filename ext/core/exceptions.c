#include "core/exceptions.h"

#include <string.h>

#define MSG_LENGTH      86

void raise_invalid_argument_type(const char *type) {
    char msg[MSG_LENGTH];

    sprintf(msg, "invalid argument, expected a %s object", type);

    rb_raise(rb_eArgError, "%s", msg);
}

void raise_invalid_argument_class(VALUE rb_cKlass) {
    raise_invalid_argument_type(RSTRING_PTR(rb_funcall(rb_cKlass, rb_intern("name"), 0)));
}

void raise_invalid_arguments_excepted(int expected, size_t given) {
    char msg[MSG_LENGTH];

    if (expected > 0) {
        sprintf(msg, "wrong number of arguments (given %lu, expected %i)", given, expected);
    } else {
        sprintf(msg, "wrong number of arguments (given %lu)", given);
    }


    rb_raise(rb_eArgError, "%s", msg);
}

void raise_invalid_array_length(size_t length) {
    char msg[MSG_LENGTH];

    sprintf(msg, "invalid array length, expected length of %lu", length);

    rb_raise(rb_eArgError, "%s", msg);
}

void raise_method_no_implemented(const char *method) {
    if (method != NULL && strlen(method) > 0) {
        rb_raise(rb_eNotImpError, "method not implemented: %s", method);
    } else {
        rb_raise(rb_eNotImpError, "method not implemented");
    }
}
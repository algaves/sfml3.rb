#include "ext/rect.h"

#include <stdio.h>
#include <stdlib.h>

#include <ext/sfml.h>
#include <ext/module.h>

VALUE Rect_new(float x, float y, float width, float height) {
    VALUE arr;

    arr = rb_ary_new();

    rb_ary_store(arr, 0, DBL2NUM(x));
    rb_ary_store(arr, 1, DBL2NUM(y));
    rb_ary_store(arr, 2, DBL2NUM(width));
    rb_ary_store(arr, 3, DBL2NUM(height));

    return arr;
}

void Rect_check(VALUE rb_arr) {
    if (rb_array_len(rb_arr) != 4) {
        rb_raise(rb_eArgError, "invalid length of array");
    }
}
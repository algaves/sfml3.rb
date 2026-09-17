#include "core/unicode.h"

#include <ruby.h>
#include <ruby/encoding.h>

VALUE utf32_from_rb(VALUE rb_string) {
    rb_encoding* utf8 = rb_utf8_encoding();
    VALUE string = rb_str_export_to_enc(rb_string, utf8);
    const char* cursor = RSTRING_PTR(string);
    const char* end = cursor + RSTRING_LEN(string);
    VALUE buffer;
    sfChar32* out;
    long length = 0;

    /* One code point per byte is the worst case, plus the terminator. */
    buffer = rb_str_tmp_new((RSTRING_LEN(string) + 1) * (long)sizeof(sfChar32));
    out = (sfChar32*)RSTRING_PTR(buffer);

    while (cursor < end) {
        int width = 0;

        out[length++] = (sfChar32)rb_enc_codepoint_len(cursor, end, &width, utf8);
        cursor += width;
    }

    out[length] = 0;

    RB_GC_GUARD(string);

    return buffer;
}

VALUE utf32_to_rb(const sfChar32* string) {
    VALUE out = rb_utf8_str_new_cstr("");
    size_t i;

    if (string == NULL) {
        return out;
    }

    for (i = 0; string[i] != 0; i++) {
        rb_str_concat(out, UINT2NUM(string[i]));
    }

    return out;
}

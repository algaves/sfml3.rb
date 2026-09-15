#include "window/clipboard.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/sfml.h"

static void unicode_char_to_utf8(sfChar32 code, char buffer[4], int *length) {
    if (code < 0x80) {
        buffer[0] = (char) code;
        *length = 1;
    } else if (code < 0x800) {
        buffer[0] = (char) (0xC0 | (code >> 6));
        buffer[1] = (char) (0x80 | (code & 0x3F));
        *length = 2;
    } else if (code < 0x10000) {
        buffer[0] = (char) (0xE0 | (code >> 12));
        buffer[1] = (char) (0x80 | ((code >> 6) & 0x3F));
        buffer[2] = (char) (0x80 | (code & 0x3F));
        *length = 3;
    } else {
        buffer[0] = (char) (0xF0 | (code >> 18));
        buffer[1] = (char) (0x80 | ((code >> 12) & 0x3F));
        buffer[2] = (char) (0x80 | ((code >> 6) & 0x3F));
        buffer[3] = (char) (0x80 | (code & 0x3F));
        *length = 4;
    }
}

static VALUE Clipboard_get_string(VALUE module) {
    const char *string = sfClipboard_getString();

    return rb_utf8_str_new_cstr(string != NULL ? string : "");
}

static VALUE Clipboard_set_string(VALUE module, VALUE rb_text) {
    sfClipboard_setString(StringValueCStr(rb_text));
    return rb_text;
}

static VALUE Clipboard_get_unicode_string(VALUE module) {
    const sfChar32 *chars = sfClipboard_getUnicodeString();
    VALUE string = rb_utf8_str_new("", 0);

    if (chars == NULL) {
        return string;
    }

    for (size_t i = 0; chars[i] != 0; i++) {
        char buffer[4];
        int length;

        unicode_char_to_utf8(chars[i], buffer, &length);
        rb_str_cat(string, buffer, length);
    }

    return string;
}

static VALUE Clipboard_set_unicode_string(VALUE module, VALUE rb_text) {
    VALUE codepoints;
    long count;
    sfChar32 *chars;

    StringValue(rb_text);
    codepoints = rb_funcall(rb_text, rb_intern("codepoints"), 0);
    count = RARRAY_LEN(codepoints);
    chars = ALLOC_N(sfChar32, count + 1);

    for (long i = 0; i < count; i++) {
        chars[i] = (sfChar32) NUM2UINT(rb_ary_entry(codepoints, i));
    }

    chars[count] = 0;
    sfClipboard_setUnicodeString(chars);

    xfree(chars);

    return rb_text;
}

void Init_Clipboard(VALUE rb_module) {
    VALUE rb_mClipboard = rb_define_module_under(rb_module, "Clipboard");

    rb_define_module_function(rb_mClipboard, "string", Clipboard_get_string, 0);
    rb_define_module_function(rb_mClipboard, "string=", Clipboard_set_string, 1);
    rb_define_module_function(rb_mClipboard, "unicode_string", Clipboard_get_unicode_string, 0);
    rb_define_module_function(rb_mClipboard, "unicode_string=", Clipboard_set_unicode_string, 1);
}

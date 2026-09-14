#include "graphics/enums.h"

#include <ruby.h>
#include <string.h>

#include "core/exceptions.h"

static const char *primitive_names[] = {
    "points", "lines", "line_strip", "triangles", "triangle_strip", "triangle_fan"
};

const char *coordinate_type_name(sfCoordinateType type) {
    return (type == sfCoordinateTypePixels) ? "pixels" : "normalized";
}

sfCoordinateType coordinate_type_from_rb(VALUE rb_type) {
    if (SYMBOL_P(rb_type)) {
        const char *name = rb_id2name(SYM2ID(rb_type));

        if (strcmp(name, "pixels") == 0) {
            return sfCoordinateTypePixels;
        }

        if (strcmp(name, "normalized") == 0) {
            return sfCoordinateTypeNormalized;
        }
    }

    if (RB_INTEGER_TYPE_P(rb_type)) {
        return (sfCoordinateType) NUM2INT(rb_type);
    }

    rb_raise(rb_eArgError, "expected :normalized or :pixels");
    return sfCoordinateTypeNormalized;
}

const char *primitive_type_name(sfPrimitiveType type) {
    if (type >= sfPoints && type <= sfTriangleFan) {
        return primitive_names[type];
    }

    return "points";
}

sfPrimitiveType primitive_type_from_rb(VALUE rb_type) {
    if (SYMBOL_P(rb_type)) {
        const char *name = rb_id2name(SYM2ID(rb_type));
        size_t i;

        for (i = 0; i < sizeof(primitive_names) / sizeof(primitive_names[0]); i++) {
            if (strcmp(name, primitive_names[i]) == 0) {
                return (sfPrimitiveType) i;
            }
        }
    }

    if (RB_INTEGER_TYPE_P(rb_type)) {
        return (sfPrimitiveType) NUM2INT(rb_type);
    }

    rb_raise(rb_eArgError, "expected a primitive type symbol");
    return sfPoints;
}

VALUE text_style_to_rb(uint32_t style) {
    VALUE array = rb_ary_new();

    if ((style & sfTextBold) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("bold")));
    }

    if ((style & sfTextItalic) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("italic")));
    }

    if ((style & sfTextUnderlined) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("underlined")));
    }

    if ((style & sfTextStrikeThrough) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("strike_through")));
    }

    return array;
}

uint32_t text_style_from_rb(VALUE rb_style) {
    uint32_t style = 0;

    if (SYMBOL_P(rb_style) || RB_TYPE_P(rb_style, T_STRING)) {
        rb_style = rb_ary_new_from_args(1, rb_style);
    }

    if (RB_INTEGER_TYPE_P(rb_style)) {
        return (uint32_t) NUM2UINT(rb_style);
    }

    if (!RB_TYPE_P(rb_style, T_ARRAY)) {
        rb_raise(rb_eArgError, "expected a text style symbol or array of symbols");
    }

    for (long i = 0; i < RARRAY_LEN(rb_style); i++) {
        const char *name = rb_id2name(SYM2ID(rb_ary_entry(rb_style, i)));

        if (strcmp(name, "regular") == 0) {
            continue;
        }

        if (strcmp(name, "bold") == 0) {
            style |= sfTextBold;
        } else if (strcmp(name, "italic") == 0) {
            style |= sfTextItalic;
        } else if (strcmp(name, "underlined") == 0 || strcmp(name, "underline") == 0) {
            style |= sfTextUnderlined;
        } else if (strcmp(name, "strike_through") == 0 || strcmp(name, "strikethrough") == 0) {
            style |= sfTextStrikeThrough;
        } else {
            rb_raise(rb_eArgError, "unknown text style: %s", name);
        }
    }

    return style;
}

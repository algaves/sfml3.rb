#include "graphics/color.h"

#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfColor color;
} Color;

static VALUE rb_cColor;

static void Color_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t Color_data_type = {
    .wrap_struct_name = "SFML::Color",
    .function = {.dmark = NULL, .dfree = Color_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static unsigned char clamp_channel(int value) {
    if (value < 0) {
        return 0;
    }

    if (value > 255) {
        return 255;
    }

    return (unsigned char) value;
}

static VALUE Color_wrap(sfColor color) {
    Color *ptr = malloc(sizeof(Color));

    ptr->color = color;

    return TypedData_Wrap_Struct(rb_cColor, &Color_data_type, ptr);
}

static sfColor Color_from_integer(VALUE rb_integer) {
    unsigned int value = (unsigned int) NUM2UINT(rb_integer);

    if (value <= 0xFFFFFF) {
        return (sfColor) {
            (unsigned char) ((value >> 16) & 0xFF),
            (unsigned char) ((value >> 8) & 0xFF),
            (unsigned char) (value & 0xFF),
            255
        };
    }

    return (sfColor) {
        (unsigned char) ((value >> 24) & 0xFF),
        (unsigned char) ((value >> 16) & 0xFF),
        (unsigned char) ((value >> 8) & 0xFF),
        (unsigned char) (value & 0xFF)
    };
}

static VALUE Color_new(int argc, VALUE *argv, VALUE klass) {
    VALUE self;
    Color *ptr;
    sfColor color = {0, 0, 0, 255};

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cColor)) {
        color = ((Color *) Get_Color_Struct(argv[0]))->color;
    } else if (argc == 1 && RB_TYPE_P(argv[0], T_ARRAY)) {
        color = color_from_rb(argv[0]);
    } else if (argc == 1 && RB_INTEGER_TYPE_P(argv[0])) {
        color = Color_from_integer(argv[0]);
    } else if (argc == 3 || argc == 4) {
        color.r = clamp_channel(NUM2INT(argv[0]));
        color.g = clamp_channel(NUM2INT(argv[1]));
        color.b = clamp_channel(NUM2INT(argv[2]));
        color.a = (argc == 4) ? clamp_channel(NUM2INT(argv[3])) : 255;
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(4, argc);
    }

    ptr = malloc(sizeof(Color));
    ptr->color = color;

    self = TypedData_Wrap_Struct(klass, &Color_data_type, ptr);

    return self;
}

static VALUE Color_from_rgb(VALUE klass, VALUE rb_integer) {
    return Color_wrap((sfColor) {
        (unsigned char) ((NUM2UINT(rb_integer) >> 16) & 0xFF),
        (unsigned char) ((NUM2UINT(rb_integer) >> 8) & 0xFF),
        (unsigned char) (NUM2UINT(rb_integer) & 0xFF),
        255
    });
}

static VALUE Color_from_rgba(VALUE klass, VALUE rb_integer) {
    return Color_wrap(Color_from_integer(rb_integer));
}

static VALUE Color_get_r(VALUE self) {
    return INT2NUM(((Color *) Get_Color_Struct(self))->color.r);
}

static VALUE Color_get_g(VALUE self) {
    return INT2NUM(((Color *) Get_Color_Struct(self))->color.g);
}

static VALUE Color_get_b(VALUE self) {
    return INT2NUM(((Color *) Get_Color_Struct(self))->color.b);
}

static VALUE Color_get_a(VALUE self) {
    return INT2NUM(((Color *) Get_Color_Struct(self))->color.a);
}

static VALUE Color_set_r(VALUE self, VALUE rb_value) {
    ((Color *) Get_Color_Struct(self))->color.r = clamp_channel(NUM2INT(rb_value));
    return rb_value;
}

static VALUE Color_set_g(VALUE self, VALUE rb_value) {
    ((Color *) Get_Color_Struct(self))->color.g = clamp_channel(NUM2INT(rb_value));
    return rb_value;
}

static VALUE Color_set_b(VALUE self, VALUE rb_value) {
    ((Color *) Get_Color_Struct(self))->color.b = clamp_channel(NUM2INT(rb_value));
    return rb_value;
}

static VALUE Color_set_a(VALUE self, VALUE rb_value) {
    ((Color *) Get_Color_Struct(self))->color.a = clamp_channel(NUM2INT(rb_value));
    return rb_value;
}

static VALUE Color_to_i(VALUE self) {
    sfColor color = ((Color *) Get_Color_Struct(self))->color;

    return UINT2NUM(((unsigned int) color.r << 24) | ((unsigned int) color.g << 16) |
                    ((unsigned int) color.b << 8) | (unsigned int) color.a);
}

static VALUE Color_to_a(VALUE self) {
    sfColor color = ((Color *) Get_Color_Struct(self))->color;

    return rb_ary_new_from_args(4, INT2NUM(color.r), INT2NUM(color.g), INT2NUM(color.b), INT2NUM(color.a));
}

static VALUE Color_eql(VALUE self, VALUE rb_other) {
    sfColor a = ((Color *) Get_Color_Struct(self))->color;
    sfColor b;

    if (!rb_obj_is_kind_of(rb_other, rb_cColor)) {
        return Qfalse;
    }

    b = ((Color *) Get_Color_Struct(rb_other))->color;

    return BOOL2RB(a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}

static VALUE Color_add(VALUE self, VALUE rb_other) {
    sfColor a = ((Color *) Get_Color_Struct(self))->color;
    sfColor b = color_from_rb(rb_other);

    return Color_wrap((sfColor) {
        clamp_channel(a.r + b.r), clamp_channel(a.g + b.g),
        clamp_channel(a.b + b.b), clamp_channel(a.a + b.a)
    });
}

static VALUE Color_sub(VALUE self, VALUE rb_other) {
    sfColor a = ((Color *) Get_Color_Struct(self))->color;
    sfColor b = color_from_rb(rb_other);

    return Color_wrap((sfColor) {
        clamp_channel(a.r - b.r), clamp_channel(a.g - b.g),
        clamp_channel(a.b - b.b), clamp_channel(a.a - b.a)
    });
}

static VALUE Color_mul(VALUE self, VALUE rb_other) {
    sfColor a = ((Color *) Get_Color_Struct(self))->color;
    sfColor b = color_from_rb(rb_other);

    return Color_wrap((sfColor) {
        (unsigned char) ((a.r * b.r) / 255), (unsigned char) ((a.g * b.g) / 255),
        (unsigned char) ((a.b * b.b) / 255), (unsigned char) ((a.a * b.a) / 255)
    });
}

static VALUE Color_to_s(VALUE self) {
    sfColor c = ((Color *) Get_Color_Struct(self))->color;
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "(%u, %u, %u, %u)", c.r, c.g, c.b, c.a);

    return rb_str_new2(buffer);
}

static void Color_define_predefined(VALUE klass, const char *name, sfColor color) {
    rb_define_const(klass, name, Color_wrap(color));
}

void Init_Color(VALUE rb_module) {
    rb_cColor = rb_define_class_under(rb_module, "Color", rb_cObject);

    rb_define_singleton_method(rb_cColor, "new", Color_new, -1);
    rb_define_singleton_method(rb_cColor, "from_rgb", Color_from_rgb, 1);
    rb_define_singleton_method(rb_cColor, "from_rgba", Color_from_rgba, 1);
    rb_define_singleton_method(rb_cColor, "from_integer", Color_from_rgba, 1);

    rb_define_method(rb_cColor, "r", Color_get_r, 0);
    rb_define_method(rb_cColor, "g", Color_get_g, 0);
    rb_define_method(rb_cColor, "b", Color_get_b, 0);
    rb_define_method(rb_cColor, "a", Color_get_a, 0);
    rb_define_method(rb_cColor, "r=", Color_set_r, 1);
    rb_define_method(rb_cColor, "g=", Color_set_g, 1);
    rb_define_method(rb_cColor, "b=", Color_set_b, 1);
    rb_define_method(rb_cColor, "a=", Color_set_a, 1);

    rb_define_method(rb_cColor, "to_i", Color_to_i, 0);
    rb_define_method(rb_cColor, "to_a", Color_to_a, 0);
    rb_define_method(rb_cColor, "to_ary", Color_to_a, 0);
    rb_define_method(rb_cColor, "==", Color_eql, 1);
    rb_define_method(rb_cColor, "+", Color_add, 1);
    rb_define_method(rb_cColor, "-", Color_sub, 1);
    rb_define_method(rb_cColor, "*", Color_mul, 1);
    rb_define_method(rb_cColor, "to_s", Color_to_s, 0);

    Color_define_predefined(rb_cColor, "BLACK", sfBlack);
    Color_define_predefined(rb_cColor, "WHITE", sfWhite);
    Color_define_predefined(rb_cColor, "RED", sfRed);
    Color_define_predefined(rb_cColor, "GREEN", sfGreen);
    Color_define_predefined(rb_cColor, "BLUE", sfBlue);
    Color_define_predefined(rb_cColor, "YELLOW", sfYellow);
    Color_define_predefined(rb_cColor, "MAGENTA", sfMagenta);
    Color_define_predefined(rb_cColor, "CYAN", sfCyan);
    Color_define_predefined(rb_cColor, "TRANSPARENT", sfTransparent);
}

VALUE Get_Klass_Color(void) {
    return rb_cColor;
}

void *Get_Color_Struct(VALUE self) {
    Color *ptr;
    TypedData_Get_Struct(self, Color, &Color_data_type, ptr);
    return ptr;
}

sfColor color_from_rb(VALUE rb_color) {
    if (rb_obj_is_kind_of(rb_color, rb_cColor)) {
        return ((Color *) Get_Color_Struct(rb_color))->color;
    }

    if (RB_TYPE_P(rb_color, T_ARRAY)) {
        long length = RARRAY_LEN(rb_color);

        if (length != 3 && length != 4) {
            raise_invalid_array_length(VALID_LENGTH_COLOR);
        }

        return (sfColor) {
            clamp_channel(NUM2INT(rb_ary_entry(rb_color, 0))),
            clamp_channel(NUM2INT(rb_ary_entry(rb_color, 1))),
            clamp_channel(NUM2INT(rb_ary_entry(rb_color, 2))),
            (length == 4) ? clamp_channel(NUM2INT(rb_ary_entry(rb_color, 3))) : 255
        };
    }

    if (RB_INTEGER_TYPE_P(rb_color)) {
        return Color_from_integer(rb_color);
    }

    raise_invalid_argument_class(rb_cColor);

    return (sfColor) {0, 0, 0, 255};
}

VALUE color_to_rb(sfColor color) {
    return Color_wrap(color);
}

VALUE color_new(int r, int g, int b, int a) {
    return Color_wrap((sfColor) {clamp_channel(r), clamp_channel(g), clamp_channel(b), clamp_channel(a)});
}

void color_check(VALUE rb_color) {
    if (rb_obj_is_kind_of(rb_color, rb_cColor) || RB_TYPE_P(rb_color, T_ARRAY) || RB_INTEGER_TYPE_P(rb_color)) {
        return;
    }

    raise_invalid_argument_class(rb_cColor);
}

void color_swap(sfColor *color_a, sfColor *color_b) {
    sfColor c;

    c = *color_b;
    *color_b = *color_a;
    *color_a = c;
}

sfColor color_new_from_rb(VALUE rb_color) {
    return color_from_rb(rb_color);
}

VALUE color_new_from_c(sfColor color) {
    return Color_wrap(color);
}

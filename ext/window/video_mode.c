#include "window/video_mode.h"

#include <ruby.h>
#include <stdlib.h>

#include "system/vec2.h"
#include "core/sfml.h"
#include "core/macros.h"

static VALUE rb_cMode;

static sfVideoMode *VideoMode_alloc(unsigned width, unsigned height, unsigned bits, int *created) {
    sfVideoMode *mode = malloc(sizeof(sfVideoMode));

    if (created != NULL) {
        *created = 1;
    }

    mode->size.x = width;
    mode->size.y = height;
    mode->bitsPerPixel = bits;

    return mode;
}

static void VideoMode_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t VideoMode_data_type = {
    .wrap_struct_name = "SFML::VideoMode",
    .function = {.dmark = NULL, .dfree = VideoMode_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE VideoMode_from_c(sfVideoMode mode) {
    sfVideoMode *ptr = VideoMode_alloc(mode.size.x, mode.size.y, mode.bitsPerPixel, NULL);
    VALUE self = TypedData_Wrap_Struct(rb_cMode, &VideoMode_data_type, ptr);

    rb_iv_set(self, "@width", UINT2NUM(mode.size.x));
    rb_iv_set(self, "@height", UINT2NUM(mode.size.y));
    rb_iv_set(self, "@bits", UINT2NUM(mode.bitsPerPixel));

    return self;
}

static VALUE VideoMode_new(VALUE klass, VALUE rb_width, VALUE rb_height, VALUE rb_bits) {
    VALUE self;
    VALUE argv[] = {rb_width, rb_height, rb_bits};

    self = VideoMode_from_c((sfVideoMode) {
        {(unsigned) NUM2UINT(rb_width), (unsigned) NUM2UINT(rb_height)},
        (unsigned) NUM2UINT(rb_bits)
    });

    rb_obj_call_init(self, 3, argv);

    return self;
}

static VALUE VideoMode_init(VALUE self, VALUE rb_width, VALUE rb_height, VALUE rb_bits) {
    rb_iv_set(self, "@width", rb_width);
    rb_iv_set(self, "@height", rb_height);
    rb_iv_set(self, "@bits", rb_bits);

    return self;
}

static VALUE VideoMode_desktop_mode(VALUE klass) {
    return VideoMode_from_c(sfVideoMode_getDesktopMode());
}

static VALUE VideoMode_fullscreen_modes(VALUE klass) {
    size_t count = 0;
    const sfVideoMode *modes = sfVideoMode_getFullscreenModes(&count);
    VALUE array = rb_ary_new_capa((long) count);

    for (size_t i = 0; i < count; i++) {
        rb_ary_push(array, VideoMode_from_c(modes[i]));
    }

    return array;
}

static VALUE VideoMode_is_available(VALUE self) {
    return BOOL2RB(sfVideoMode_isValid(*Get_Mode_Struct(self)));
}

static VALUE VideoMode_get_size(VALUE self) {
    sfVideoMode *mode = Get_Mode_Struct(self);

    return vec2f_to_rb((sfVector2f) {(float) mode->size.x, (float) mode->size.y});
}

static VALUE VideoMode_eql(VALUE self, VALUE rb_other) {
    sfVideoMode a = *Get_Mode_Struct(self);
    sfVideoMode b;

    if (!rb_obj_is_kind_of(rb_other, rb_cMode)) {
        return Qfalse;
    }

    b = *Get_Mode_Struct(rb_other);

    return BOOL2RB(a.size.x == b.size.x && a.size.y == b.size.y && a.bitsPerPixel == b.bitsPerPixel);
}

void Init_VideoMode(VALUE rb_module) {
    rb_cMode = rb_define_class_under(rb_module, "VideoMode", rb_cObject);

    rb_define_singleton_method(rb_cMode, "new", VideoMode_new, 3);
    rb_define_singleton_method(rb_cMode, "desktop_mode", VideoMode_desktop_mode, 0);
    rb_define_singleton_method(rb_cMode, "fullscreen_modes", VideoMode_fullscreen_modes, 0);

    rb_define_method(rb_cMode, "initialize", VideoMode_init, 3);
    rb_define_method(rb_cMode, "available?", VideoMode_is_available, 0);
    rb_define_method(rb_cMode, "valid?", VideoMode_is_available, 0);
    rb_define_method(rb_cMode, "size", VideoMode_get_size, 0);
    rb_define_method(rb_cMode, "==", VideoMode_eql, 1);
    rb_define_attr(rb_cMode, "width", 1, 0);
    rb_define_attr(rb_cMode, "height", 1, 0);
    rb_define_attr(rb_cMode, "bits", 1, 0);
}

sfVideoMode *Get_Mode_Struct(VALUE self) {
    sfVideoMode *ptr;
    TypedData_Get_Struct(self, sfVideoMode, &VideoMode_data_type, ptr);
    return ptr;
}

VALUE Get_Klass_Mode() {
    return rb_cMode;
}

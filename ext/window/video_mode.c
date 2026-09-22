#include "window/video_mode.h"

#include <ruby.h>
#include <stdlib.h>

#include "system/vec2.h"
#include "core/sfml.h"
#include "core/macros.h"

static VALUE rb_cMode;

static sfVideoMode* VideoMode_create(unsigned width, unsigned height, unsigned bits, int* created) {
    sfVideoMode* mode = malloc(sizeof(sfVideoMode));

    if (mode == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate video mode");
    }

    if (created != NULL) {
        *created = 1;
    }

    mode->size.x = width;
    mode->size.y = height;
    mode->bitsPerPixel = bits;

    return mode;
}

static void VideoMode_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t VideoMode_data_type = {
    .wrap_struct_name = "SF::Window::VideoMode",
    .function = {.dmark = NULL, .dfree = VideoMode_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE VideoMode_alloc(VALUE klass) {
    sfVideoMode* ptr = VideoMode_create(0, 0, 0, NULL);

    return TypedData_Wrap_Struct(klass, &VideoMode_data_type, ptr);
}

static VALUE VideoMode_from_c(sfVideoMode mode) {
    sfVideoMode* ptr = VideoMode_create(mode.size.x, mode.size.y, mode.bitsPerPixel, NULL);
    VALUE self = TypedData_Wrap_Struct(rb_cMode, &VideoMode_data_type, ptr);

    rb_iv_set(self, "@width", UINT2NUM(mode.size.x));
    rb_iv_set(self, "@height", UINT2NUM(mode.size.y));
    rb_iv_set(self, "@bits", UINT2NUM(mode.bitsPerPixel));

    return self;
}

/* call-seq:
 *   VideoMode.new(width, height, bits_per_pixel) -> VideoMode
 *
 * Creates a video mode with the given +width+, +height+ and +bits_per_pixel+.
 *
 * @return [VideoMode]
 */
static VALUE VideoMode_initialize(VALUE self, VALUE rb_width, VALUE rb_height, VALUE rb_bits) {
    sfVideoMode* mode = Get_Mode_Struct(self);

    mode->size.x = (unsigned)NUM2UINT(rb_width);
    mode->size.y = (unsigned)NUM2UINT(rb_height);
    mode->bitsPerPixel = (unsigned)NUM2UINT(rb_bits);

    rb_iv_set(self, "@width", rb_width);
    rb_iv_set(self, "@height", rb_height);
    rb_iv_set(self, "@bits", rb_bits);

    return self;
}

/* call-seq: desktop_mode -> VideoMode
 *
 * Returns the current desktop video mode.
 *
 * @return [VideoMode] the current desktop video mode
 */
static VALUE VideoMode_desktop_mode(VALUE klass) {
    return VideoMode_from_c(sfVideoMode_getDesktopMode());
}

/* call-seq: fullscreen_modes -> Array<VideoMode>
 *
 * Returns all fullscreen video modes supported by the current desktop.
 *
 * @return [Array<VideoMode>] all video modes supported in fullscreen mode,
 *   sorted from best to worst
 */
static VALUE VideoMode_fullscreen_modes(VALUE klass) {
    size_t count = 0;
    const sfVideoMode* modes = sfVideoMode_getFullscreenModes(&count);
    VALUE array = rb_ary_new_capa((long)count);

    for (size_t i = 0; i < count; i++) {
        rb_ary_push(array, VideoMode_from_c(modes[i]));
    }

    return array;
}

/* call-seq: valid? -> true or false
 *
 * Returns +true+ if this mode is valid for fullscreen use.
 *
 * @return [Boolean] whether this mode is valid for fullscreen use on the
 *   current desktop
 */
static VALUE VideoMode_is_available(VALUE self) {
    return BOOL2RB(sfVideoMode_isValid(*Get_Mode_Struct(self)));
}

/* call-seq: size -> Vector2
 *
 * Returns +width+ and +height+ as a Vector2.
 *
 * @return [Vector2] +width+ and +height+ as a vector
 */
static VALUE VideoMode_get_size(VALUE self) {
    sfVideoMode* mode = Get_Mode_Struct(self);

    return vec2f_to_rb((sfVector2f){(float)mode->size.x, (float)mode->size.y});
}

/* call-seq:
 *   self == other -> true or false
 *
 * Returns +true+ if +other+ has the same size and bit depth.
 *
 * @return [Boolean]
 */
static VALUE VideoMode_eql(VALUE self, VALUE rb_other) {
    sfVideoMode a = *Get_Mode_Struct(self);
    sfVideoMode b;

    if (!rb_obj_is_kind_of(rb_other, rb_cMode)) {
        return Qfalse;
    }

    b = *Get_Mode_Struct(rb_other);

    return BOOL2RB(a.size.x == b.size.x && a.size.y == b.size.y &&
                   a.bitsPerPixel == b.bitsPerPixel);
}

/* Document-class: SF::Window::VideoMode
 * A width/height/bits-per-pixel triple describing a display mode, as used
 * for fullscreen windows.
 *
 * The +width+/+height+/+bits+ accessors below are declared with
 * +rb_define_attr+ rather than +rb_define_method+; YARD's C parser cannot
 * handle that declaration form (a confirmed YARD bug), so they are
 * documented here instead of above their (nonexistent, ivar-backed)
 * definitions.
 *
 * @!attribute [r] width
 *   Width in pixels.
 *   @return [Integer] width in pixels
 * @!attribute [r] height
 *   Height in pixels.
 *   @return [Integer] height in pixels
 * @!attribute [r] bits
 *   Bits per pixel.
 *   @return [Integer] bits per pixel
 */
void Init_VideoMode(VALUE rb_mWindow) {
    rb_cMode = rb_define_class_under(rb_mWindow, "VideoMode", rb_cObject);

    rb_define_alloc_func(rb_cMode, VideoMode_alloc);
    rb_define_method(rb_cMode, "initialize", VideoMode_initialize, 3);

    rb_define_singleton_method(rb_cMode, "desktop_mode", VideoMode_desktop_mode, 0);
    rb_define_singleton_method(rb_cMode, "fullscreen_modes", VideoMode_fullscreen_modes, 0);
    rb_define_method(rb_cMode, "available?", VideoMode_is_available, 0);
    rb_define_method(rb_cMode, "valid?", VideoMode_is_available, 0);
    rb_define_method(rb_cMode, "size", VideoMode_get_size, 0);
    rb_define_method(rb_cMode, "==", VideoMode_eql, 1);
    rb_define_attr(rb_cMode, "width", 1, 0);
    rb_define_attr(rb_cMode, "height", 1, 0);
    rb_define_attr(rb_cMode, "bits", 1, 0);
}

sfVideoMode* Get_Mode_Struct(VALUE self) {
    sfVideoMode* ptr;
    TypedData_Get_Struct(self, sfVideoMode, &VideoMode_data_type, ptr);
    return ptr;
}

VALUE Get_Klass_Mode() {
    return rb_cMode;
}

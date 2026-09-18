#include "graphics/image.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/color.h"
#include "graphics/rect.h"
#include "system/buffer.h"
#include "system/input_stream.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cImage;

static void Image_free(void* ptr) {
    sfImage_destroy(ptr);
}

static const rb_data_type_t Image_data_type = {
    .wrap_struct_name = "SFML::Image",
    .function = {.dmark = NULL, .dfree = Image_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Image_wrap(VALUE klass, sfImage* image) {
    if (image == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create image");
    }

    return TypedData_Wrap_Struct(klass, &Image_data_type, image);
}

/* call-seq:
 *   Image.new(size) -> Image
 *
 * Creates a black, fully-opaque image of the given size. +size+ is a
 * Vector2 or 2-element Array.
 *
 * @return [Image]
 * @raise [RuntimeError] if the image cannot be created
 */
static VALUE Image_new(VALUE klass, VALUE rb_size) {
    return Image_wrap(klass, sfImage_create(vec2u_from_rb(rb_size)));
}

/* call-seq:
 *   Image.from_color(size, color) -> Image
 *
 * @return [Image]
 * @raise [RuntimeError] if the image cannot be created
 */
static VALUE Image_from_color(VALUE klass, VALUE rb_size, VALUE rb_color) {
    return Image_wrap(klass,
                      sfImage_createFromColor(vec2u_from_rb(rb_size), color_from_rb(rb_color)));
}

/* call-seq:
 *   Image.from_pixels(size, pixels) -> Image
 *
 * +pixels+ is a String of raw RGBA bytes, at least +width * height * 4+
 * bytes long.
 *
 * @return [Image]
 * @raise [ArgumentError] if +pixels+ is shorter than required
 * @raise [RuntimeError] if the image cannot be created
 */
static VALUE Image_from_pixels(VALUE klass, VALUE rb_size, VALUE rb_pixels) {
    sfVector2u size = vec2u_from_rb(rb_size);
    size_t expected = (size_t)size.x * size.y * 4;
    sfImage* image;

    StringValue(rb_pixels);

    if ((size_t)RSTRING_LEN(rb_pixels) < expected) {
        rb_raise(rb_eArgError, "pixel data too short: expected %lu bytes", (unsigned long)expected);
    }

    image = sfImage_createFromPixels(size, (const uint8_t*)RSTRING_PTR(rb_pixels));

    return Image_wrap(klass, image);
}

/* call-seq:
 *   Image.from_file(path) -> Image
 *
 * @return [Image]
 * @raise [RuntimeError] if the file cannot be loaded
 */
static VALUE Image_from_file(VALUE klass, VALUE rb_path) {
    return Image_wrap(klass, sfImage_createFromFile(StringValueCStr(rb_path)));
}

/* call-seq:
 *   Image.from_memory(data) -> Image
 *
 * @return [Image]
 * @raise [RuntimeError] if the image data cannot be decoded
 */
static VALUE Image_from_memory(VALUE klass, VALUE rb_data) {
    StringValue(rb_data);

    return Image_wrap(klass,
                      sfImage_createFromMemory(RSTRING_PTR(rb_data), (size_t)RSTRING_LEN(rb_data)));
}

/* call-seq:
 *   Image.from_stream(stream) -> Image
 *
 * +stream+ is any object responding to the InputStream protocol (see
 * InputStream).
 *
 * @return [Image]
 * @raise [RuntimeError] if the image cannot be loaded
 */
static VALUE Image_from_stream(VALUE klass, VALUE rb_stream) {
    VALUE holder = Qnil;
    sfInputStream* stream = input_stream_from_rb(rb_stream, &holder);

    (void)holder;

    return Image_wrap(klass, sfImage_createFromStream(stream));
}

/* call-seq: copy -> Image
 *
 * @return [Image] an independent copy
 */
static VALUE Image_copy(VALUE self) {
    return Image_wrap(Get_Klass_Image(), sfImage_copy(Get_Image_Struct(self)));
}

/* call-seq: size -> Vector2
 *
 * @return [Vector2] the image's dimensions in pixels
 */
static VALUE Image_get_size(VALUE self) {
    return vec2f_to_rb((sfVector2f){(float)sfImage_getSize(Get_Image_Struct(self)).x,
                                    (float)sfImage_getSize(Get_Image_Struct(self)).y});
}

/* call-seq: save_to_file(path) -> true or false
 *
 * The format is deduced from +path+'s extension.
 *
 * @return [Boolean] whether the save succeeded
 */
static VALUE Image_save_to_file(VALUE self, VALUE rb_path) {
    return BOOL2RB(sfImage_saveToFile(Get_Image_Struct(self), StringValueCStr(rb_path)));
}

/* call-seq:
 *   save_to_memory(format = "png") -> Buffer or nil
 *
 * +format+ is an image file extension without the dot (e.g. +"png"+,
 * +"bmp"+, +"tga"+, +"jpg"+).
 *
 * @return [Buffer, nil] the encoded image data, or +nil+ if encoding failed
 * @raise [ArgumentError] if given more than one argument
 */
static VALUE Image_save_to_memory(int argc, VALUE* argv, VALUE self) {
    VALUE rb_buffer;
    const char* format = "png";
    if (argc > 1) {
        raise_invalid_arguments_excepted(1, argc);
    }

    if (argc == 1 && !NIL_P(argv[0])) {
        format = StringValueCStr(argv[0]);
    }

    rb_buffer = rb_funcall(Get_Klass_Buffer(), rb_intern("new"), 0);

    if (sfImage_saveToMemory(Get_Image_Struct(self), Get_Buffer_Struct(rb_buffer), format)) {
        return rb_buffer;
    }

    return Qnil;
}

/* call-seq: create_mask_from_color(color, alpha) -> self
 *
 * Sets the alpha channel of every pixel matching +color+ to +alpha+.
 *
 * @return [self]
 */
static VALUE Image_create_mask_from_color(VALUE self, VALUE rb_color, VALUE rb_alpha) {
    sfImage_createMaskFromColor((sfImage*)Get_Image_Struct(self), color_from_rb(rb_color),
                                (uint8_t)NUM2INT(rb_alpha));
    return self;
}

/* call-seq:
 *   copy_image(source, dest, source_rect = Rect.new, apply_alpha = true) -> true or false
 *
 * Copies pixels from +source+ (an Image) onto +self+ at position +dest+.
 * +source_rect+ restricts the copied region; an empty/zero rect copies the
 * whole source image.
 *
 * @return [Boolean] whether the copy succeeded
 */
static VALUE Image_copy_image(int argc, VALUE* argv, VALUE self) {
    VALUE rb_source, rb_dest, rb_source_rect, rb_apply_alpha;
    bool apply_alpha = true;

    rb_scan_args(argc, argv, "31", &rb_source, &rb_dest, &rb_source_rect, &rb_apply_alpha);

    if (!rb_obj_is_kind_of(rb_source, rb_cImage)) {
        raise_invalid_argument_class(rb_cImage);
    }

    if (!NIL_P(rb_apply_alpha)) {
        apply_alpha = RTEST(rb_apply_alpha);
    }

    return BOOL2RB(sfImage_copyImage((sfImage*)Get_Image_Struct(self), Get_Image_Struct(rb_source),
                                     vec2u_from_rb(rb_dest), int_rect_from_rb(rb_source_rect),
                                     apply_alpha));
}

/* call-seq: pixel(x, y) -> Color
 *
 * @return [Color] the color of the pixel at (+x+, +y+)
 */
static VALUE Image_get_pixel(VALUE self, VALUE rb_x, VALUE rb_y) {
    sfColor color = sfImage_getPixel(
        Get_Image_Struct(self), (sfVector2u){(unsigned)NUM2UINT(rb_x), (unsigned)NUM2UINT(rb_y)});

    return color_to_rb(color);
}

/* call-seq: set_pixel(x, y, color) -> self
 *
 * @return [self]
 */
static VALUE Image_set_pixel(VALUE self, VALUE rb_x, VALUE rb_y, VALUE rb_color) {
    sfImage_setPixel((sfImage*)Get_Image_Struct(self),
                     (sfVector2u){(unsigned)NUM2UINT(rb_x), (unsigned)NUM2UINT(rb_y)},
                     color_from_rb(rb_color));
    return self;
}

/* call-seq: pixels -> String
 *
 * @return [String] the raw RGBA pixel data, +width * height * 4+ bytes,
 *   row-major starting from the top-left
 */
static VALUE Image_get_pixels(VALUE self) {
    const sfImage* image = Get_Image_Struct(self);
    sfVector2u size = sfImage_getSize(image);
    const uint8_t* pixels = sfImage_getPixelsPtr(image);

    if (pixels == NULL) {
        return rb_str_new("", 0);
    }

    return rb_str_new((const char*)pixels, (long)(size.x * size.y * 4));
}

/* call-seq: flip_horizontally! -> self
 *
 * @return [self]
 */
static VALUE Image_flip_horizontally(VALUE self) {
    sfImage_flipHorizontally((sfImage*)Get_Image_Struct(self));
    return self;
}

/* call-seq: flip_vertically! -> self
 *
 * @return [self]
 */
static VALUE Image_flip_vertically(VALUE self) {
    sfImage_flipVertically((sfImage*)Get_Image_Struct(self));
    return self;
}

/* Document-class: SFML::Image
 * A 2D array of RGBA pixels, held in RAM (as opposed to Texture, which is
 * GPU-side). Used for loading, editing and saving raw pixel data.
 */
void Init_Image(VALUE rb_mSFML) {
    rb_cImage = rb_define_class_under(rb_mSFML, "Image", rb_cObject);

    rb_define_singleton_method(rb_cImage, "new", Image_new, 1);
    rb_define_singleton_method(rb_cImage, "from_color", Image_from_color, 2);
    rb_define_singleton_method(rb_cImage, "from_pixels", Image_from_pixels, 2);
    rb_define_singleton_method(rb_cImage, "from_file", Image_from_file, 1);
    rb_define_singleton_method(rb_cImage, "from_memory", Image_from_memory, 1);
    rb_define_singleton_method(rb_cImage, "from_stream", Image_from_stream, 1);

    rb_define_method(rb_cImage, "copy", Image_copy, 0);
    rb_define_method(rb_cImage, "size", Image_get_size, 0);
    rb_define_method(rb_cImage, "save_to_file", Image_save_to_file, 1);
    rb_define_method(rb_cImage, "save_to_memory", Image_save_to_memory, -1);
    rb_define_method(rb_cImage, "create_mask_from_color", Image_create_mask_from_color, 2);
    rb_define_method(rb_cImage, "copy_image", Image_copy_image, -1);
    rb_define_method(rb_cImage, "pixel", Image_get_pixel, 2);
    rb_define_method(rb_cImage, "set_pixel", Image_set_pixel, 3);
    rb_define_method(rb_cImage, "pixels", Image_get_pixels, 0);
    rb_define_method(rb_cImage, "flip_horizontally!", Image_flip_horizontally, 0);
    rb_define_method(rb_cImage, "flip_vertically!", Image_flip_vertically, 0);
}

VALUE Get_Klass_Image(void) {
    return rb_cImage;
}

const sfImage* Get_Image_Struct(VALUE self) {
    sfImage* ptr;
    TypedData_Get_Struct(self, sfImage, &Image_data_type, ptr);
    return ptr;
}

VALUE image_from_c(sfImage* image) {
    return Image_wrap(Get_Klass_Image(), image);
}

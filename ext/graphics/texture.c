#include "graphics/texture.h"

#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "graphics/image.h"
#include "graphics/rect.h"
#include "graphics/enums.h"
#include "system/input_stream.h"
#include "system/vec2.h"
#include "window/window.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cTexture;

typedef struct {
    sfTexture* texture;
    bool owns;
} Texture;

static void Texture_free(void* ptr) {
    Texture* texture = ptr;

    if (texture->owns) {
        sfTexture_destroy(texture->texture);
    }

    free(texture);
}

static const rb_data_type_t Texture_data_type = {
    .wrap_struct_name = "SFML::Texture",
    .function = {.dmark = NULL, .dfree = Texture_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Texture_wrap(VALUE klass, sfTexture* texture) {
    Texture* ptr;

    if (texture == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create texture");
    }

    ptr = malloc(sizeof(Texture));
    ptr->texture = texture;
    ptr->owns = true;

    return TypedData_Wrap_Struct(klass, &Texture_data_type, ptr);
}

VALUE texture_from_borrowed(const sfTexture* texture) {
    Texture* ptr;

    if (texture == NULL) {
        return Qnil;
    }

    ptr = malloc(sizeof(Texture));
    ptr->texture = (sfTexture*)texture;
    ptr->owns = false;

    return TypedData_Wrap_Struct(rb_cTexture, &Texture_data_type, ptr);
}

/* Area stays NULL unless an argument was given, mirroring CSFML's "whole
   texture" convention. The pointer stays valid for the call only. */
static const sfIntRect* Texture_area_ptr(VALUE rb_area, sfIntRect* storage) {
    if (NIL_P(rb_area)) {
        return NULL;
    }

    *storage = int_rect_from_rb(rb_area);

    return storage;
}

static VALUE Texture_initialize_copy(VALUE self, VALUE other) {
    (void)other;
    rb_raise(rb_eTypeError, "can't copy a %s", rb_obj_classname(self));
}

static VALUE Texture_alloc(VALUE klass) {
    Texture* ptr = malloc(sizeof(Texture));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate texture");
    }

    ptr->texture = NULL;
    ptr->owns = false;

    return TypedData_Wrap_Struct(klass, &Texture_data_type, ptr);
}

/* call-seq:
 *   Texture.new(size) -> Texture
 *
 * Creates an empty texture with the given +width+ and +height+.
 *
 * @return [Texture] an empty texture of +size+ (a Vector2 or 2-element Array)
 * @raise [RuntimeError] if creation fails
 */
static VALUE Texture_initialize(VALUE self, VALUE rb_size) {
    Texture* ptr;
    sfTexture* texture = sfTexture_create(vec2u_from_rb(rb_size));

    if (texture == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create texture");
    }

    TypedData_Get_Struct(self, Texture, &Texture_data_type, ptr);
    ptr->texture = texture;
    ptr->owns = true;

    return self;
}

/* call-seq:
 *   Texture.srgb(size) -> Texture
 *
 * Creates an empty texture with an sRGB format.
 *
 * @return [Texture] an empty sRGB-encoded texture of +size+
 * @raise [RuntimeError] if creation fails
 */
static VALUE Texture_srgb(VALUE klass, VALUE rb_size) {
    return Texture_wrap(klass, sfTexture_createSrgb(vec2u_from_rb(rb_size)));
}

/* call-seq:
 *   Texture.from_file(path)       -> Texture
 *   Texture.from_file(path, area) -> Texture
 *
 * Loads a texture from the image file at +filename+.
 *
 * @return [Texture] loaded from the file at +path+, cropped to +area+ (a
 *   Rect) if given
 * @raise [RuntimeError] if loading fails
 */
static VALUE Texture_from_file(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_path, rb_area;
    sfIntRect area;

    rb_scan_args(argc, argv, "11", &rb_path, &rb_area);

    return Texture_wrap(klass, sfTexture_createFromFile(StringValueCStr(rb_path),
                                                        Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.srgb_from_file(path)       -> Texture
 *   Texture.srgb_from_file(path, area) -> Texture
 *
 * Loads an sRGB texture from the image file at +filename+.
 *
 * @return [Texture] sRGB-encoded, loaded from the file at +path+, cropped
 *   to +area+ (a Rect) if given
 * @raise [RuntimeError] if loading fails
 */
static VALUE Texture_srgb_from_file(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_path, rb_area;
    sfIntRect area;

    rb_scan_args(argc, argv, "11", &rb_path, &rb_area);

    return Texture_wrap(klass, sfTexture_createSrgbFromFile(StringValueCStr(rb_path),
                                                            Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.from_memory(data)       -> Texture
 *   Texture.from_memory(data, area) -> Texture
 *
 * Loads a texture from an in-memory image buffer.
 *
 * @return [Texture] decoded from the encoded image bytes in +data+ (a
 *   String), cropped to +area+ (a Rect) if given
 * @raise [RuntimeError] if decoding fails
 */
static VALUE Texture_from_memory(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_data, rb_area;
    sfIntRect area;

    rb_scan_args(argc, argv, "11", &rb_data, &rb_area);
    StringValue(rb_data);

    return Texture_wrap(klass, sfTexture_createFromMemory(RSTRING_PTR(rb_data),
                                                          (size_t)RSTRING_LEN(rb_data),
                                                          Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.srgb_from_memory(data)       -> Texture
 *   Texture.srgb_from_memory(data, area) -> Texture
 *
 * Loads an sRGB texture from an in-memory image buffer.
 *
 * @return [Texture] sRGB-encoded, decoded from the encoded image bytes in
 *   +data+ (a String), cropped to +area+ (a Rect) if given
 * @raise [RuntimeError] if decoding fails
 */
static VALUE Texture_srgb_from_memory(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_data, rb_area;
    sfIntRect area;

    rb_scan_args(argc, argv, "11", &rb_data, &rb_area);
    StringValue(rb_data);

    return Texture_wrap(klass, sfTexture_createSrgbFromMemory(RSTRING_PTR(rb_data),
                                                              (size_t)RSTRING_LEN(rb_data),
                                                              Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.from_stream(stream)       -> Texture
 *   Texture.from_stream(stream, area) -> Texture
 *
 * Loads a texture from the image data read from +stream+.
 *
 * @return [Texture] decoded from +stream+ (an InputStream), cropped to
 *   +area+ (a Rect) if given
 * @raise [RuntimeError] if decoding fails
 */
static VALUE Texture_from_stream(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_stream, rb_area, holder = Qnil;
    sfIntRect area;
    sfInputStream* stream;

    if (argc < 1 || argc > 2) {
        raise_invalid_arguments_excepted(2, argc);
    }

    rb_stream = argv[0];
    rb_area = (argc == 2) ? argv[1] : Qnil;
    stream = input_stream_from_rb(rb_stream, &holder);

    (void)holder;

    return Texture_wrap(klass,
                        sfTexture_createFromStream(stream, Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.srgb_from_stream(stream)       -> Texture
 *   Texture.srgb_from_stream(stream, area) -> Texture
 *
 * Loads an sRGB texture from the image data read from +stream+.
 *
 * @return [Texture] sRGB-encoded, decoded from +stream+ (an InputStream),
 *   cropped to +area+ (a Rect) if given
 * @raise [RuntimeError] if decoding fails
 */
static VALUE Texture_srgb_from_stream(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_stream, rb_area, holder = Qnil;
    sfIntRect area;
    sfInputStream* stream;

    if (argc < 1 || argc > 2) {
        raise_invalid_arguments_excepted(2, argc);
    }

    rb_stream = argv[0];
    rb_area = (argc == 2) ? argv[1] : Qnil;
    stream = input_stream_from_rb(rb_stream, &holder);

    (void)holder;

    return Texture_wrap(klass,
                        sfTexture_createSrgbFromStream(stream, Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.from_image(image)       -> Texture
 *   Texture.from_image(image, area) -> Texture
 *
 * Creates a texture from the pixels of an existing Image.
 *
 * @return [Texture] built from +image+ (an Image), cropped to +area+ (a
 *   Rect) if given
 * @raise [TypeError] if +image+ is not an Image
 * @raise [RuntimeError] if creation fails
 */
static VALUE Texture_from_image(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_image, rb_area;
    sfIntRect area;

    rb_scan_args(argc, argv, "11", &rb_image, &rb_area);

    if (!rb_obj_is_kind_of(rb_image, Get_Klass_Image())) {
        raise_invalid_argument_class(Get_Klass_Image());
    }

    return Texture_wrap(klass, sfTexture_createFromImage(Get_Image_Struct(rb_image),
                                                         Texture_area_ptr(rb_area, &area)));
}

/* call-seq:
 *   Texture.srgb_from_image(image)       -> Texture
 *   Texture.srgb_from_image(image, area) -> Texture
 *
 * Creates an sRGB texture from the pixels of an existing Image.
 *
 * @return [Texture] sRGB-encoded, built from +image+ (an Image), cropped to
 *   +area+ (a Rect) if given
 * @raise [TypeError] if +image+ is not an Image
 * @raise [RuntimeError] if creation fails
 */
static VALUE Texture_srgb_from_image(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_image, rb_area;
    sfIntRect area;

    rb_scan_args(argc, argv, "11", &rb_image, &rb_area);

    if (!rb_obj_is_kind_of(rb_image, Get_Klass_Image())) {
        raise_invalid_argument_class(Get_Klass_Image());
    }

    return Texture_wrap(klass, sfTexture_createSrgbFromImage(Get_Image_Struct(rb_image),
                                                             Texture_area_ptr(rb_area, &area)));
}

/* call-seq: copy -> Texture
 *
 * Returns a deep copy of the object.
 *
 * @return [Texture] an independent copy
 */
static VALUE Texture_copy(VALUE self) {
    return Texture_wrap(Get_Klass_Texture(), sfTexture_copy(Get_Texture_Struct(self)));
}

/* call-seq: size -> Vector2
 *
 * Returns the object's size.
 *
 * @return [Vector2]
 */
static VALUE Texture_get_size(VALUE self) {
    sfVector2u size = sfTexture_getSize(Get_Texture_Struct(self));

    return vec2f_to_rb((sfVector2f){(float)size.x, (float)size.y});
}

/* call-seq:
 *   resize(size) -> true or false
 *
 * Resizes the texture in place. Existing pixel data is lost.
 *
 * @return [Boolean] whether resizing succeeded
 */
static VALUE Texture_resize(VALUE self, VALUE rb_size) {
    return BOOL2RB(sfTexture_resize((sfTexture*)Get_Texture_Struct(self), vec2u_from_rb(rb_size)));
}

/* call-seq:
 *   resize_srgb(size) -> true or false
 *
 * Resizes the texture in place as sRGB-encoded. Existing pixel data is
 * lost.
 *
 * @return [Boolean] whether resizing succeeded
 */
static VALUE Texture_resize_srgb(VALUE self, VALUE rb_size) {
    return BOOL2RB(
        sfTexture_resizeSrgb((sfTexture*)Get_Texture_Struct(self), vec2u_from_rb(rb_size)));
}

/* Exchanges the two textures' contents in place, so anything already holding
   either object sees the swap. */
/* call-seq:
 *   swap(other) -> self
 *
 * Swaps the contents with +other+.
 *
 * @return [self]
 * @raise [TypeError] if +other+ is not a Texture
 */
static VALUE Texture_swap(VALUE self, VALUE rb_other) {
    if (!rb_obj_is_kind_of(rb_other, Get_Klass_Texture())) {
        raise_invalid_argument_class(Get_Klass_Texture());
    }

    sfTexture_swap((sfTexture*)Get_Texture_Struct(self), (sfTexture*)Get_Texture_Struct(rb_other));

    return self;
}

/* call-seq: copy_to_image -> Image
 *
 * Copies the texture's pixels into +image+.
 *
 * @return [Image] the texture's pixels, read back from the GPU
 * @raise [RuntimeError] if the copy fails
 */
static VALUE Texture_copy_to_image(VALUE self) {
    sfImage* image = sfTexture_copyToImage(Get_Texture_Struct(self));

    if (image == NULL) {
        rb_raise(rb_eRuntimeError, "failed to copy texture to image");
    }

    return image_from_c(image);
}

/* call-seq:
 *   update_from_pixels(pixels, size, offset) -> self
 *
 * Overwrites a +size+ region of the texture at +offset+ with +pixels+ (a
 * String of raw RGBA8 bytes, +size.x * size.y * 4+ long).
 *
 * @return [self]
 * @raise [ArgumentError] if +pixels+ is shorter than required
 */
static VALUE Texture_update_from_pixels(VALUE self, VALUE rb_pixels, VALUE rb_size,
                                        VALUE rb_offset) {
    sfVector2u size = vec2u_from_rb(rb_size);
    sfVector2u offset = vec2u_from_rb(rb_offset);
    size_t expected;

    /* size.x * size.y * 4 as a bare size_t multiplication can wrap on the
       32-bit targets this gem ships; check before multiplying rather than
       after so a wrapped-to-near-zero "expected" can't slip a too-short
       buffer past the length check below (see the identical fix in
       graphics/image.c). */
    if (size.x != 0 && size.y > (SIZE_MAX / 4) / size.x) {
        rb_raise(rb_eArgError, "texture dimensions too large");
    }
    expected = (size_t)size.x * size.y * 4;

    StringValue(rb_pixels);

    if ((size_t)RSTRING_LEN(rb_pixels) < expected) {
        rb_raise(rb_eArgError, "pixel data too short");
    }

    sfTexture_updateFromPixels((sfTexture*)Get_Texture_Struct(self),
                               (const uint8_t*)RSTRING_PTR(rb_pixels), size, offset);

    return self;
}

/* call-seq:
 *   update_from_image(image, offset) -> self
 *
 * Updates the texture from the pixels of +image+.
 *
 * @return [self]
 * @raise [TypeError] if +image+ is not an Image
 */
static VALUE Texture_update_from_image(VALUE self, VALUE rb_image, VALUE rb_offset) {
    if (!rb_obj_is_kind_of(rb_image, Get_Klass_Image())) {
        raise_invalid_argument_class(Get_Klass_Image());
    }

    sfTexture_updateFromImage((sfTexture*)Get_Texture_Struct(self), Get_Image_Struct(rb_image),
                              vec2u_from_rb(rb_offset));

    return self;
}

/* call-seq:
 *   update_from_texture(source, offset) -> self
 *
 * Updates the texture from the pixels of another texture.
 *
 * @return [self]
 * @raise [TypeError] if +source+ is not a Texture
 */
static VALUE Texture_update_from_texture(VALUE self, VALUE rb_source, VALUE rb_offset) {
    if (!rb_obj_is_kind_of(rb_source, rb_cTexture)) {
        raise_invalid_argument_class(rb_cTexture);
    }

    sfTexture_updateFromTexture((sfTexture*)Get_Texture_Struct(self), Get_Texture_Struct(rb_source),
                                vec2u_from_rb(rb_offset));

    return self;
}

/* call-seq:
 *   update_from_window(window, offset) -> self
 *
 * Updates the texture from the contents of a window.
 *
 * @return [self]
 * @raise [TypeError] if +window+ is not a Window
 */
static VALUE Texture_update_from_window(VALUE self, VALUE rb_window, VALUE rb_offset) {
    if (!rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        raise_invalid_argument_class(Get_Klass_Window());
    }

    sfTexture_updateFromRenderWindow((sfTexture*)Get_Texture_Struct(self),
                                     Get_Window_Struct(rb_window), vec2u_from_rb(rb_offset));

    return self;
}

/* call-seq:
 *   smooth=(value) -> true or false
 *
 * Enables or disables smooth rendering.
 *
 * @return [Boolean] +value+
 */
static VALUE Texture_set_smooth(VALUE self, VALUE rb_smooth) {
    sfTexture_setSmooth((sfTexture*)Get_Texture_Struct(self), RTEST(rb_smooth));
    return rb_smooth;
}

/* call-seq: smooth? -> true or false
 *
 * Returns +true+ if smooth rendering is enabled.
 *
 * @return [Boolean]
 */
static VALUE Texture_is_smooth(VALUE self) {
    return BOOL2RB(sfTexture_isSmooth(Get_Texture_Struct(self)));
}

/* call-seq: srgb? -> true or false
 *
 * Returns +true+ if the texture uses an sRGB format.
 *
 * @return [Boolean]
 */
static VALUE Texture_is_srgb(VALUE self) {
    return BOOL2RB(sfTexture_isSrgb(Get_Texture_Struct(self)));
}

/* call-seq:
 *   repeated=(value) -> true or false
 *
 * Enables or disables texture repeating.
 *
 * @return [Boolean] +value+
 */
static VALUE Texture_set_repeated(VALUE self, VALUE rb_repeated) {
    sfTexture_setRepeated((sfTexture*)Get_Texture_Struct(self), RTEST(rb_repeated));
    return rb_repeated;
}

/* call-seq: repeated? -> true or false
 *
 * Returns +true+ if texture repeating is enabled.
 *
 * @return [Boolean]
 */
static VALUE Texture_is_repeated(VALUE self) {
    return BOOL2RB(sfTexture_isRepeated(Get_Texture_Struct(self)));
}

/* call-seq: generate_mipmap -> true or false
 *
 * Generates the mipmap pyramid for the texture.
 *
 * @return [Boolean] whether mipmap generation succeeded
 */
static VALUE Texture_generate_mipmap(VALUE self) {
    return BOOL2RB(sfTexture_generateMipmap((sfTexture*)Get_Texture_Struct(self)));
}

/* call-seq: native_handle -> Integer
 *
 * Returns the underlying OpenGL handle.
 *
 * @return [Integer] the underlying OpenGL texture handle
 */
static VALUE Texture_get_native_handle(VALUE self) {
    return UINT2NUM(sfTexture_getNativeHandle(Get_Texture_Struct(self)));
}

/* call-seq:
 *   bind                        -> self
 *   bind(coordinate_type)       -> self
 *
 * Activates this texture for rendering. +coordinate_type+ selects
 * normalized (0..1) or pixel texture coordinates and defaults to
 * normalized.
 *
 * @return [self]
 */
static VALUE Texture_bind(int argc, VALUE* argv, VALUE self) {
    VALUE rb_coordinate_type;
    sfCoordinateType type = sfCoordinateTypeNormalized;

    rb_scan_args(argc, argv, "01", &rb_coordinate_type);

    if (!NIL_P(rb_coordinate_type)) {
        type = coordinate_type_from_rb(rb_coordinate_type);
    }

    sfTexture_bind(Get_Texture_Struct(self), type);

    return self;
}

/* call-seq:
 *   Texture.maximum_size -> Integer
 *
 * Returns the maximum texture size supported by the GPU.
 *
 * @return [Integer] the maximum texture size supported by the current
 *   OpenGL implementation
 */
static VALUE Texture_maximum_size(VALUE klass) {
    return UINT2NUM(sfTexture_getMaximumSize());
}

/* Document-class: SFML::Texture
 * An image living in GPU memory, usable for rendering (Sprite, Text,
 * Shape) or as a render target (RenderTexture#texture).
 *
 * #smooth?/#smooth=, #srgb?, and #repeated?/#repeated= control sampling;
 * see each method for details.
 */
void Init_Texture(VALUE rb_mSFML) {
    rb_cTexture = rb_define_class_under(rb_mSFML, "Texture", rb_cObject);

    rb_define_alloc_func(rb_cTexture, Texture_alloc);
    rb_define_method(rb_cTexture, "initialize", Texture_initialize, 1);
    rb_define_private_method(rb_cTexture, "initialize_copy", Texture_initialize_copy, 1);
    rb_define_singleton_method(rb_cTexture, "from_file", Texture_from_file, -1);
    rb_define_singleton_method(rb_cTexture, "from_memory", Texture_from_memory, -1);
    rb_define_singleton_method(rb_cTexture, "from_stream", Texture_from_stream, -1);
    rb_define_singleton_method(rb_cTexture, "from_image", Texture_from_image, -1);
    rb_define_singleton_method(rb_cTexture, "srgb", Texture_srgb, 1);
    rb_define_singleton_method(rb_cTexture, "srgb_from_file", Texture_srgb_from_file, -1);
    rb_define_singleton_method(rb_cTexture, "srgb_from_memory", Texture_srgb_from_memory, -1);
    rb_define_singleton_method(rb_cTexture, "srgb_from_stream", Texture_srgb_from_stream, -1);
    rb_define_singleton_method(rb_cTexture, "srgb_from_image", Texture_srgb_from_image, -1);
    rb_define_singleton_method(rb_cTexture, "maximum_size", Texture_maximum_size, 0);

    rb_define_method(rb_cTexture, "copy", Texture_copy, 0);
    rb_define_method(rb_cTexture, "size", Texture_get_size, 0);
    rb_define_method(rb_cTexture, "resize", Texture_resize, 1);
    rb_define_method(rb_cTexture, "resize_srgb", Texture_resize_srgb, 1);
    rb_define_method(rb_cTexture, "swap", Texture_swap, 1);
    rb_define_method(rb_cTexture, "copy_to_image", Texture_copy_to_image, 0);
    rb_define_method(rb_cTexture, "update_from_pixels", Texture_update_from_pixels, 3);
    rb_define_method(rb_cTexture, "update_from_image", Texture_update_from_image, 2);
    rb_define_method(rb_cTexture, "update_from_texture", Texture_update_from_texture, 2);
    rb_define_method(rb_cTexture, "update_from_window", Texture_update_from_window, 2);
    rb_define_method(rb_cTexture, "smooth=", Texture_set_smooth, 1);
    rb_define_method(rb_cTexture, "smooth?", Texture_is_smooth, 0);
    rb_define_method(rb_cTexture, "srgb?", Texture_is_srgb, 0);
    rb_define_method(rb_cTexture, "repeated=", Texture_set_repeated, 1);
    rb_define_method(rb_cTexture, "repeated?", Texture_is_repeated, 0);
    rb_define_method(rb_cTexture, "generate_mipmap", Texture_generate_mipmap, 0);
    rb_define_method(rb_cTexture, "native_handle", Texture_get_native_handle, 0);
    rb_define_method(rb_cTexture, "bind", Texture_bind, -1);
}

VALUE Get_Klass_Texture(void) {
    return rb_cTexture;
}

const sfTexture* Get_Texture_Struct(VALUE self) {
    Texture* ptr;
    TypedData_Get_Struct(self, Texture, &Texture_data_type, ptr);
    return ptr->texture;
}

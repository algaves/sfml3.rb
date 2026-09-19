#include "graphics/font.h"

#include <ruby.h>

#include "graphics/glyph.h"
#include "graphics/texture.h"
#include "system/input_stream.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cFont;

static void Font_free(void* ptr) {
    sfFont_destroy(ptr);
}

static const rb_data_type_t Font_data_type = {
    .wrap_struct_name = "SFML::Font",
    .function = {.dmark = NULL, .dfree = Font_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Font_wrap(VALUE klass, sfFont* font) {
    if (font == NULL) {
        rb_raise(rb_eRuntimeError, "failed to load font");
    }

    return TypedData_Wrap_Struct(klass, &Font_data_type, font);
}

/* call-seq:
 *   Font.from_file(path) -> Font
 *
 * Loads a font from the font file at +filename+.
 *
 * @return [Font]
 * @raise [RuntimeError] if the file cannot be loaded
 */
static VALUE Font_from_file(VALUE klass, VALUE rb_path) {
    return Font_wrap(klass, sfFont_createFromFile(StringValueCStr(rb_path)));
}

/* call-seq:
 *   Font.from_memory(data) -> Font
 *
 * Loads a font from an in-memory font file buffer.
 *
 * @return [Font]
 * @raise [RuntimeError] if the font data cannot be parsed
 */
static VALUE Font_from_memory(VALUE klass, VALUE rb_data) {
    StringValue(rb_data);

    return Font_wrap(klass,
                     sfFont_createFromMemory(RSTRING_PTR(rb_data), (size_t)RSTRING_LEN(rb_data)));
}

/* call-seq:
 *   Font.from_stream(stream) -> Font
 *
 * +stream+ is any object responding to the InputStream protocol (see
 * InputStream).
 *
 * @return [Font]
 * @raise [RuntimeError] if the font cannot be loaded
 */
static VALUE Font_from_stream(VALUE klass, VALUE rb_stream) {
    VALUE holder = Qnil;
    sfInputStream* stream = input_stream_from_rb(rb_stream, &holder);

    (void)holder;

    return Font_wrap(klass, sfFont_createFromStream(stream));
}

/* call-seq: copy -> Font
 *
 * Returns a deep copy of the object.
 *
 * @return [Font] an independent copy
 */
static VALUE Font_copy(VALUE self) {
    return Font_wrap(Get_Klass_Font(), sfFont_copy(Get_Font_Struct(self)));
}

/* call-seq:
 *   glyph(codepoint, size, bold=false, outline_thickness=0) -> Glyph
 *
 * Returns the glyph for +codepoint+ at the given +character_size+.
 *
 * @return [Glyph]
 */
static VALUE Font_get_glyph(int argc, VALUE* argv, VALUE self) {
    VALUE rb_codepoint, rb_size, rb_bold, rb_outline;
    bool bold = false;
    float outline = 0;

    rb_scan_args(argc, argv, "22", &rb_codepoint, &rb_size, &rb_bold, &rb_outline);

    if (!NIL_P(rb_bold)) {
        bold = RTEST(rb_bold);
    }

    if (!NIL_P(rb_outline)) {
        outline = NUM2DBL(rb_outline);
    }

    return glyph_from_c(sfFont_getGlyph(Get_Font_Struct(self), (uint32_t)NUM2UINT(rb_codepoint),
                                        (unsigned int)NUM2UINT(rb_size), bold, outline));
}

/* call-seq: has_glyph?(codepoint) -> true or false
 *
 * Returns +true+ if the font contains a glyph for +codepoint+.
 *
 * @return [Boolean] whether this font has a glyph for the given Unicode
 *   codepoint
 */
static VALUE Font_has_glyph(VALUE self, VALUE rb_codepoint) {
    return BOOL2RB(sfFont_hasGlyph(Get_Font_Struct(self), (uint32_t)NUM2UINT(rb_codepoint)));
}

/* call-seq: kerning(first, second, size) -> Float
 *
 * Returns the kerning between two code points at the given size.
 *
 * @return [Float] the kerning offset between two consecutive glyphs at the
 *   given character size
 */
static VALUE Font_get_kerning(VALUE self, VALUE rb_first, VALUE rb_second, VALUE rb_size) {
    return DBL2NUM(sfFont_getKerning(Get_Font_Struct(self), (uint32_t)NUM2UINT(rb_first),
                                     (uint32_t)NUM2UINT(rb_second),
                                     (unsigned int)NUM2UINT(rb_size)));
}

/* call-seq: bold_kerning(first, second, size) -> Float
 *
 * Returns the kerning of the bold variant between two code points.
 *
 * @return [Float] the kerning offset between two consecutive bold glyphs at
 *   the given character size
 */
static VALUE Font_get_bold_kerning(VALUE self, VALUE rb_first, VALUE rb_second, VALUE rb_size) {
    return DBL2NUM(sfFont_getBoldKerning(Get_Font_Struct(self), (uint32_t)NUM2UINT(rb_first),
                                         (uint32_t)NUM2UINT(rb_second),
                                         (unsigned int)NUM2UINT(rb_size)));
}

/* call-seq: line_spacing(size) -> Float
 *
 * Returns the line spacing for the given character size.
 *
 * @return [Float] the distance between two consecutive lines at the given
 *   character size
 */
static VALUE Font_get_line_spacing(VALUE self, VALUE rb_size) {
    return DBL2NUM(sfFont_getLineSpacing(Get_Font_Struct(self), (unsigned int)NUM2UINT(rb_size)));
}

/* call-seq: underline_position(size) -> Float
 *
 * Returns the position of the underline for the given size.
 *
 * @return [Float] the position of the underline, relative to the baseline,
 *   at the given character size
 */
static VALUE Font_get_underline_position(VALUE self, VALUE rb_size) {
    return DBL2NUM(
        sfFont_getUnderlinePosition(Get_Font_Struct(self), (unsigned int)NUM2UINT(rb_size)));
}

/* call-seq: underline_thickness(size) -> Float
 *
 * Returns the thickness of the underline for the given size.
 *
 * @return [Float] the thickness of the underline at the given character size
 */
static VALUE Font_get_underline_thickness(VALUE self, VALUE rb_size) {
    return DBL2NUM(
        sfFont_getUnderlineThickness(Get_Font_Struct(self), (unsigned int)NUM2UINT(rb_size)));
}

/* call-seq: texture(size) -> Texture
 *
 * Returns the texture atlas holding rendered glyphs for the given size.
 *
 * @return [Texture] the texture atlas holding rendered glyphs for the given
 *   character size. Owned by the font; do not modify or free it directly.
 */
static VALUE Font_get_texture(VALUE self, VALUE rb_size) {
    return texture_from_borrowed(
        sfFont_getTexture((sfFont*)Get_Font_Struct(self), (unsigned int)NUM2UINT(rb_size)));
}

/* call-seq:
 *   smooth=(value) -> Boolean
 *
 * Enables or disables smooth rendering.
 *
 * @return [Boolean] +value+
 */
static VALUE Font_set_smooth(VALUE self, VALUE rb_smooth) {
    sfFont_setSmooth((sfFont*)Get_Font_Struct(self), RTEST(rb_smooth));
    return rb_smooth;
}

/* call-seq: smooth? -> true or false
 *
 * Returns +true+ if smooth rendering is enabled.
 *
 * @return [Boolean]
 */
static VALUE Font_is_smooth(VALUE self) {
    return BOOL2RB(sfFont_isSmooth(Get_Font_Struct(self)));
}

/* call-seq: info -> String
 *
 * Returns the font's family name.
 *
 * @return [String] the font family name
 */
static VALUE Font_get_info(VALUE self) {
    sfFontInfo info = sfFont_getInfo(Get_Font_Struct(self));

    return rb_str_new_cstr(info.family != NULL ? info.family : "");
}

/* Document-class: SFML::Font
 * A font face used to render text, loaded from a file, memory buffer or
 * stream. Glyphs are rasterized and cached lazily per character size.
 */
void Init_Font(VALUE rb_mSFML) {
    rb_cFont = rb_define_class_under(rb_mSFML, "Font", rb_cObject);

    rb_define_singleton_method(rb_cFont, "from_file", Font_from_file, 1);
    rb_define_singleton_method(rb_cFont, "from_memory", Font_from_memory, 1);
    rb_define_singleton_method(rb_cFont, "from_stream", Font_from_stream, 1);

    rb_define_method(rb_cFont, "copy", Font_copy, 0);
    rb_define_method(rb_cFont, "glyph", Font_get_glyph, -1);
    rb_define_method(rb_cFont, "has_glyph?", Font_has_glyph, 1);
    rb_define_method(rb_cFont, "kerning", Font_get_kerning, 3);
    rb_define_method(rb_cFont, "bold_kerning", Font_get_bold_kerning, 3);
    rb_define_method(rb_cFont, "line_spacing", Font_get_line_spacing, 1);
    rb_define_method(rb_cFont, "underline_position", Font_get_underline_position, 1);
    rb_define_method(rb_cFont, "underline_thickness", Font_get_underline_thickness, 1);
    rb_define_method(rb_cFont, "texture", Font_get_texture, 1);
    rb_define_method(rb_cFont, "smooth=", Font_set_smooth, 1);
    rb_define_method(rb_cFont, "smooth?", Font_is_smooth, 0);
    rb_define_method(rb_cFont, "info", Font_get_info, 0);
}

VALUE Get_Klass_Font(void) {
    return rb_cFont;
}

const sfFont* Get_Font_Struct(VALUE self) {
    sfFont* ptr;
    TypedData_Get_Struct(self, sfFont, &Font_data_type, ptr);
    return ptr;
}

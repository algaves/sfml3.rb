#include "graphics/glyph.h"

#include <ruby.h>
#include <stdlib.h>

#include "graphics/rect.h"

typedef struct {
    sfGlyph glyph;
} Glyph;

static VALUE rb_cGlyph;

static void Glyph_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Glyph_data_type = {
    .wrap_struct_name = "SF::Graphics::Glyph",
    .function = {.dmark = NULL, .dfree = Glyph_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

VALUE glyph_from_c(sfGlyph glyph) {
    Glyph* ptr = malloc(sizeof(Glyph));

    ptr->glyph = glyph;

    return TypedData_Wrap_Struct(rb_cGlyph, &Glyph_data_type, ptr);
}

static Glyph* Get_Glyph(VALUE self) {
    Glyph* ptr;
    TypedData_Get_Struct(self, Glyph, &Glyph_data_type, ptr);
    return ptr;
}

/* call-seq: advance -> Float
 *
 * Returns the glyph's horizontal advance.
 *
 * @return [Float] the horizontal offset to advance to the next character
 */
static VALUE Glyph_get_advance(VALUE self) {
    return DBL2NUM(Get_Glyph(self)->glyph.advance);
}

/* call-seq: bounds -> Rect
 *
 * Returns the glyph's bounding box.
 *
 * @return [Rect] the glyph's bounding box, relative to the baseline and
 *   the cursor position
 */
static VALUE Glyph_get_bounds(VALUE self) {
    return rect_to_rb(Get_Glyph(self)->glyph.bounds);
}

/* call-seq: texture_rect -> Rect
 *
 * Returns the sub-rectangle of the texture displayed on the object.
 *
 * @return [Rect] the sub-rectangle of the font's texture holding this
 *   glyph's pixels
 */
static VALUE Glyph_get_texture_rect(VALUE self) {
    return int_rect_to_rb(Get_Glyph(self)->glyph.textureRect);
}

static VALUE Glyph_alloc(VALUE klass) {
    (void)klass;
    rb_raise(rb_eNotImpError,
             "glyph objects are returned by Font#glyph and cannot be constructed directly");
}

/* Document-class: SF::Graphics::Glyph
 * A single character's rendering metrics and texture location, as returned
 * by Font#glyph. Read-only.
 */
void Init_Glyph(VALUE rb_mGraphics) {
    rb_cGlyph = rb_define_class_under(rb_mGraphics, "Glyph", rb_cObject);
    rb_define_alloc_func(rb_cGlyph, Glyph_alloc);

    rb_define_method(rb_cGlyph, "advance", Glyph_get_advance, 0);
    rb_define_method(rb_cGlyph, "bounds", Glyph_get_bounds, 0);
    rb_define_method(rb_cGlyph, "texture_rect", Glyph_get_texture_rect, 0);
}

VALUE Get_Klass_Glyph(void) {
    return rb_cGlyph;
}

#include "window/cursor.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cCursor;

static void Cursor_free(void* ptr) {
    sfCursor_destroy(ptr);
}

static const rb_data_type_t Cursor_data_type = {
    .wrap_struct_name = "SF::Window::Cursor",
    .function = {.dmark = NULL, .dfree = Cursor_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Cursor_wrap(sfCursor* cursor) {
    if (cursor == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create cursor");
    }

    return TypedData_Wrap_Struct(rb_cCursor, &Cursor_data_type, cursor);
}

/* call-seq:
 *   Cursor.from_pixels(pixels, size, hotspot) -> Cursor
 *
 * Creates a cursor from RGBA32 pixel data (+width * height * 4+ bytes,
 * row-major, top-to-bottom).
 *
 * @return [Cursor]
 * @raise [ArgumentError] if +pixels+ is shorter than required
 * @raise [RuntimeError] if cursor creation fails
 */
static VALUE Cursor_from_pixels(VALUE klass, VALUE rb_pixels, VALUE rb_size, VALUE rb_hotspot) {
    sfVector2u size = vec2u_from_rb(rb_size);
    size_t expected = (size_t)size.x * size.y * 4;

    StringValue(rb_pixels);

    if ((size_t)RSTRING_LEN(rb_pixels) < expected) {
        rb_raise(rb_eArgError, "pixel data too short: expected %zu bytes", expected);
    }

    return Cursor_wrap(sfCursor_createFromPixels((const uint8_t*)RSTRING_PTR(rb_pixels), size,
                                                 vec2u_from_rb(rb_hotspot)));
}

/* call-seq:
 *   Cursor.from_system(type) -> Cursor
 *
 * Creates a cursor from a native system cursor type (a Symbol, e.g.
 * +:arrow+, +:hand+, +:crosshair+; see the cursor type symbols this binding
 * accepts).
 *
 * @return [Cursor]
 * @raise [RuntimeError] if cursor creation fails
 */
static VALUE Cursor_from_system(VALUE klass, VALUE rb_type) {
    return Cursor_wrap(sfCursor_createFromSystem(cursor_type_from_rb(rb_type)));
}

static VALUE Cursor_alloc(VALUE klass) {
    (void)klass;
    rb_raise(rb_eNotImpError, "use Cursor.from_system or Cursor.from_pixels");
}

/* Document-class: SF::Window::Cursor
 * A native mouse cursor, either built from raw pixel data or one of the
 * platform's built-in system cursors. Assign to Window#cursor= to apply.
 */
void Init_Cursor(VALUE rb_mWindow) {
    rb_cCursor = rb_define_class_under(rb_mWindow, "Cursor", rb_cObject);
    rb_define_alloc_func(rb_cCursor, Cursor_alloc);

    rb_define_singleton_method(rb_cCursor, "from_pixels", Cursor_from_pixels, 3);
    rb_define_singleton_method(rb_cCursor, "from_system", Cursor_from_system, 1);
}

VALUE Get_Klass_Cursor(void) {
    return rb_cCursor;
}

const sfCursor* Get_Cursor_Struct(VALUE self) {
    sfCursor* ptr;
    TypedData_Get_Struct(self, sfCursor, &Cursor_data_type, ptr);
    return ptr;
}

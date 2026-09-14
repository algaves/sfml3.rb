#include "window/cursor.h"

#include <ruby.h>

#include "window/input_enums.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cCursor;

static void Cursor_free(void *ptr) {
    sfCursor_destroy(ptr);
}

static const rb_data_type_t Cursor_data_type = {
    .wrap_struct_name = "SFML::Cursor",
    .function = {.dmark = NULL, .dfree = Cursor_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE Cursor_wrap(sfCursor *cursor) {
    if (cursor == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create cursor");
    }

    return TypedData_Wrap_Struct(rb_cCursor, &Cursor_data_type, cursor);
}

static VALUE Cursor_from_pixels(VALUE klass, VALUE rb_pixels, VALUE rb_size, VALUE rb_hotspot) {
    StringValue(rb_pixels);

    return Cursor_wrap(sfCursor_createFromPixels((const uint8_t *) RSTRING_PTR(rb_pixels), vec2u_from_rb(rb_size),
                                                 vec2u_from_rb(rb_hotspot)));
}

static VALUE Cursor_from_system(VALUE klass, VALUE rb_type) {
    return Cursor_wrap(sfCursor_createFromSystem(cursor_type_from_rb(rb_type)));
}

void Init_Cursor(VALUE rb_module) {
    rb_cCursor = rb_define_class_under(rb_module, "Cursor", rb_cObject);

    rb_define_singleton_method(rb_cCursor, "from_pixels", Cursor_from_pixels, 3);
    rb_define_singleton_method(rb_cCursor, "from_system", Cursor_from_system, 1);
}

VALUE Get_Klass_Cursor(void) {
    return rb_cCursor;
}

const sfCursor *Get_Cursor_Struct(VALUE self) {
    sfCursor *ptr;
    TypedData_Get_Struct(self, sfCursor, &Cursor_data_type, ptr);
    return ptr;
}

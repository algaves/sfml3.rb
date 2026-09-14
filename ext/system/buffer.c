#include "system/buffer.h"

#include <ruby.h>

#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cBuffer;

static void Buffer_free(void *ptr) {
    sfBuffer_destroy(ptr);
}

static const rb_data_type_t Buffer_data_type = {
    .wrap_struct_name = "SFML::Buffer",
    .function = {.dmark = NULL, .dfree = Buffer_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE Buffer_wrap(VALUE klass, sfBuffer *buffer) {
    return TypedData_Wrap_Struct(klass, &Buffer_data_type, buffer);
}

static VALUE Buffer_new(VALUE klass) {
    VALUE self;
    sfBuffer *buffer = sfBuffer_create();

    if (buffer == NULL) {
        rb_raise(rb_eNoMemError, "could not create buffer");
    }

    self = Buffer_wrap(klass, buffer);

    return self;
}

static VALUE Buffer_get_size(VALUE self) {
    return SIZET2NUM(sfBuffer_getSize(Get_Buffer_Struct(self)));
}

static VALUE Buffer_get_data(VALUE self) {
    sfBuffer *buffer = Get_Buffer_Struct(self);
    size_t size = sfBuffer_getSize(buffer);
    const uint8_t *data = sfBuffer_getData(buffer);

    if (data == NULL || size == 0) {
        return rb_str_new("", 0);
    }

    return rb_str_new((const char *) data, (long) size);
}

static VALUE Buffer_is_empty(VALUE self) {
    return BOOL2RB(sfBuffer_getSize(Get_Buffer_Struct(self)) == 0);
}

void Init_Buffer(VALUE rb_module) {
    rb_cBuffer = rb_define_class_under(rb_module, "Buffer", rb_cObject);

    rb_define_singleton_method(rb_cBuffer, "new", Buffer_new, 0);

    rb_define_method(rb_cBuffer, "size", Buffer_get_size, 0);
    rb_define_method(rb_cBuffer, "length", Buffer_get_size, 0);
    rb_define_method(rb_cBuffer, "data", Buffer_get_data, 0);
    rb_define_method(rb_cBuffer, "to_s", Buffer_get_data, 0);
    rb_define_method(rb_cBuffer, "empty?", Buffer_is_empty, 0);
}

VALUE Get_Klass_Buffer(void) {
    return rb_cBuffer;
}

void *Get_Buffer_Struct(VALUE self) {
    sfBuffer *ptr;
    TypedData_Get_Struct(self, sfBuffer, &Buffer_data_type, ptr);
    return ptr;
}

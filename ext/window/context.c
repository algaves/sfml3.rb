#include "window/context.h"

#include <ruby.h>

#include "window/context_settings.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cContext;

static void Context_free(void *ptr) {
    sfContext_destroy(ptr);
}

static const rb_data_type_t Context_data_type = {
    .wrap_struct_name = "SFML::Context",
    .function = {.dmark = NULL, .dfree = Context_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static sfContext *Get_Context_Struct(VALUE self) {
    sfContext *ptr;
    TypedData_Get_Struct(self, sfContext, &Context_data_type, ptr);
    return ptr;
}

static VALUE Context_new(VALUE klass) {
    sfContext *context = sfContext_create();
    VALUE self;

    if (context == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create context");
    }

    self = TypedData_Wrap_Struct(klass, &Context_data_type, context);

    return self;
}

static VALUE Context_extension_available(VALUE klass, VALUE rb_name) {
    return BOOL2RB(sfContext_isExtensionAvailable(StringValueCStr(rb_name)));
}

static VALUE Context_set_active(VALUE self, VALUE rb_active) {
    return BOOL2RB(sfContext_setActive(Get_Context_Struct(self), RTEST(rb_active)));
}

static VALUE Context_get_function(VALUE klass, VALUE rb_name) {
    return ULL2NUM((unsigned long long) (uintptr_t) sfContext_getFunction(StringValueCStr(rb_name)));
}

static VALUE Context_get_settings(VALUE self) {
    return context_settings_to_rb(sfContext_getSettings(Get_Context_Struct(self)));
}

static VALUE Context_active_context_id(VALUE klass) {
    return ULL2NUM(sfContext_getActiveContextId());
}

void Init_Context(VALUE rb_module) {
    rb_cContext = rb_define_class_under(rb_module, "Context", rb_cObject);

    rb_define_singleton_method(rb_cContext, "new", Context_new, 0);
    rb_define_singleton_method(rb_cContext, "extension_available?", Context_extension_available, 1);
    rb_define_singleton_method(rb_cContext, "function", Context_get_function, 1);
    rb_define_singleton_method(rb_cContext, "active_context_id", Context_active_context_id, 0);

    rb_define_method(rb_cContext, "active=", Context_set_active, 1);
    rb_define_method(rb_cContext, "settings", Context_get_settings, 0);
}

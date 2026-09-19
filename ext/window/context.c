#include "window/context.h"

#include <ruby.h>

#include "window/context_settings.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cContext;

static void Context_free(void* ptr) {
    sfContext_destroy(ptr);
}

static const rb_data_type_t Context_data_type = {
    .wrap_struct_name = "SFML::Context",
    .function = {.dmark = NULL, .dfree = Context_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static sfContext* Get_Context_Struct(VALUE self) {
    sfContext* ptr;
    TypedData_Get_Struct(self, sfContext, &Context_data_type, ptr);
    return ptr;
}

static VALUE Context_alloc(VALUE klass) {
    sfContext* context = sfContext_create();

    if (context == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create context");
    }

    return TypedData_Wrap_Struct(klass, &Context_data_type, context);
}

/* call-seq:
 *   Context.new -> Context
 *
 * Creates and activates a new OpenGL context on the current thread.
 *
 * @return [Context]
 * @raise [RuntimeError] if context creation fails
 */
static VALUE Context_initialize(VALUE self) {
    return self;
}

/* call-seq:
 *   extension_available?(name) -> true or false
 *
 * Returns +true+ if the OpenGL extension +name+ is available in the currently
 * active context.
 *
 * @return [Boolean] whether the OpenGL extension +name+ is available in the
 *   currently active context
 */
static VALUE Context_extension_available(VALUE klass, VALUE rb_name) {
    return BOOL2RB(sfContext_isExtensionAvailable(StringValueCStr(rb_name)));
}

/* call-seq:
 *   active=(value) -> true or false
 *
 * Activates or deactivates this context as the current one on the calling
 * thread.
 *
 * @return [Boolean] whether activation succeeded
 */
static VALUE Context_set_active(VALUE self, VALUE rb_active) {
    return BOOL2RB(sfContext_setActive(Get_Context_Struct(self), RTEST(rb_active)));
}

/* call-seq:
 *   function(name) -> Integer
 *
 * Returns the address of the OpenGL function +name+ as an Integer, or 0 when
 * it is not available.
 *
 * @return [Integer] the address of the OpenGL function +name+, as a raw
 *   pointer value, or 0 if it is not available
 */
static VALUE Context_get_function(VALUE klass, VALUE rb_name) {
    return ULL2NUM((unsigned long long)(uintptr_t)sfContext_getFunction(StringValueCStr(rb_name)));
}

/* call-seq: settings -> ContextSettings
 *
 * Returns the settings this context was created with.
 *
 * @return [ContextSettings] the settings this context was created with
 */
static VALUE Context_get_settings(VALUE self) {
    return context_settings_to_rb(sfContext_getSettings(Get_Context_Struct(self)));
}

/* call-seq: active_context_id -> Integer
 *
 * Returns the unique identifier of the currently active context on the
 * calling thread, or 0 if none is active.
 *
 * @return [Integer] the unique identifier of the currently active context on
 *   the calling thread, or 0 if none is active
 */
static VALUE Context_active_context_id(VALUE klass) {
    return ULL2NUM(sfContext_getActiveContextId());
}

/* Document-class: SFML::Context
 * A raw OpenGL context, usable off-screen or on a thread without a Window.
 */
void Init_Context(VALUE rb_mSFML) {
    rb_cContext = rb_define_class_under(rb_mSFML, "Context", rb_cObject);

    rb_define_alloc_func(rb_cContext, Context_alloc);
    rb_define_method(rb_cContext, "initialize", Context_initialize, 0);

    rb_define_singleton_method(rb_cContext, "extension_available?", Context_extension_available, 1);
    rb_define_singleton_method(rb_cContext, "function", Context_get_function, 1);
    rb_define_singleton_method(rb_cContext, "active_context_id", Context_active_context_id, 0);

    rb_define_method(rb_cContext, "active=", Context_set_active, 1);
    rb_define_method(rb_cContext, "settings", Context_get_settings, 0);
}

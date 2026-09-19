#include "window/vulkan.h"

#include <ruby.h>

#include "core/macros.h"
#include "core/sfml.h"

/* call-seq:
 *   available?(require_graphics = false) -> true or false
 *
 * Returns +true+ if Vulkan is available, optionally requiring graphics support.
 *
 * @return [Boolean] whether Vulkan is available, optionally also requiring
 *   graphics-capable support when +require_graphics+ is true
 */
static VALUE Vulkan_is_available(int argc, VALUE* argv, VALUE module) {
    VALUE rb_require_graphics;
    bool require_graphics = false;

    rb_scan_args(argc, argv, "01", &rb_require_graphics);

    if (!NIL_P(rb_require_graphics)) {
        require_graphics = RTEST(rb_require_graphics);
    }

    return BOOL2RB(sfVulkan_isAvailable(require_graphics));
}

/* call-seq:
 *   function(name) -> Integer
 *
 * Returns the address of the Vulkan function or extension +name+ as an
 * Integer, or 0 when it is not available.
 *
 * @return [Integer] the address of the Vulkan function/extension +name+, as
 *   a raw pointer value, or 0 if it is not available
 */
static VALUE Vulkan_get_function(VALUE module, VALUE rb_name) {
    return ULL2NUM((unsigned long long)(uintptr_t)sfVulkan_getFunction(StringValueCStr(rb_name)));
}

/* call-seq: graphics_required_instance_extensions -> Array<String>
 *
 * Returns the Vulkan instance extensions needed to create a window surface.
 *
 * @return [Array<String>] the Vulkan instance extensions required to create
 *   a Vulkan surface for a Window
 */
static VALUE Vulkan_get_graphics_required_instance_extensions(VALUE module) {
    size_t count = 0;
    const char* const* extensions = sfVulkan_getGraphicsRequiredInstanceExtensions(&count);
    VALUE array = rb_ary_new_capa((long)count);

    for (size_t i = 0; i < count; i++) {
        rb_ary_push(array, rb_str_new_cstr(extensions[i]));
    }

    return array;
}

/* Document-module: SFML::Vulkan
 * Vulkan support queries. Vulkan handles (instances, surfaces, allocators)
 * cross this binding as plain Integers; see Window#create_vulkan_surface.
 */
void Init_Vulkan(VALUE rb_mSFML) {
    VALUE rb_mVulkan = rb_define_module_under(rb_mSFML, "Vulkan");

    rb_define_module_function(rb_mVulkan, "available?", Vulkan_is_available, -1);
    rb_define_module_function(rb_mVulkan, "function", Vulkan_get_function, 1);
    rb_define_module_function(rb_mVulkan, "graphics_required_instance_extensions",
                              Vulkan_get_graphics_required_instance_extensions, 0);
}

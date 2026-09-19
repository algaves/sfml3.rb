/* Shared method bodies for every window class (WindowBase, Window,
   RenderWindow). Included from several translation units; each defines WB_FN,
   WB_METHOD and WB_HANDLE before including this file, then undefines them
   after. Generating concrete methods per class keeps the compiler's type
   checks on the handle and on every sf* call, which a runtime dispatch through
   void* would not.

   CSFML gives sfWindowBase_* and sfRenderWindow_* the same window surface under
   two name prefixes, so WB_FN is the only thing that differs. Anything that is
   not on sfWindowBase (settings, display, setActive, vsync, framerate) lives in
   individual classes, not here. */

#include "window/window_base.h"
#include "window/event.h"
#include "window/cursor.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/unicode.h"

/* call-seq: is_open? -> true or false */
static VALUE WB_METHOD(is_open)(VALUE self) {
    return BOOL2RB(WB_FN(isOpen)(WB_HANDLE(self)));
}

/* call-seq: close! -> self */
static VALUE WB_METHOD(close)(VALUE self) {
    WB_FN(close)(WB_HANDLE(self));
    return self;
}

/* call-seq: poll_event!(event) -> true or false */
static VALUE WB_METHOD(poll_event)(VALUE self, VALUE rb_event) {
    if (!rb_obj_is_kind_of(rb_event, Get_Klass_Event())) {
        raise_invalid_argument_class(Get_Klass_Event());
    }

    return BOOL2RB(WB_FN(pollEvent)(WB_HANDLE(self), Get_Event_Struct(rb_event)));
}

/* call-seq: wait_event!(event) -> true or false */
static VALUE WB_METHOD(wait_event)(VALUE self, VALUE rb_event) {
    if (!rb_obj_is_kind_of(rb_event, Get_Klass_Event())) {
        raise_invalid_argument_class(Get_Klass_Event());
    }

    return BOOL2RB(WB_FN(waitEvent)(WB_HANDLE(self), sfTime_Zero, Get_Event_Struct(rb_event)));
}

/* call-seq: position -> Vector2 */
static VALUE WB_METHOD(get_position)(VALUE self) {
    return VEC2_C2RB(WB_FN(getPosition)(WB_HANDLE(self)));
}

/* call-seq: position=(value) -> value */
static VALUE WB_METHOD(set_position)(VALUE self, VALUE rb_arr) {
    WB_FN(setPosition)(WB_HANDLE(self), vec2i_new_from_ruby(rb_arr));
    return self;
}

/* call-seq: size -> Vector2 */
static VALUE WB_METHOD(get_size)(VALUE self) {
    return VEC2_C2RB(WB_FN(getSize)(WB_HANDLE(self)));
}

/* call-seq: size=(value) -> self */
static VALUE WB_METHOD(set_size)(VALUE self, VALUE rb_size) {
    WB_FN(setSize)(WB_HANDLE(self), vec2u_from_rb(rb_size));
    return self;
}

/* call-seq: minimum_size=(value) -> value */
static VALUE WB_METHOD(set_minimum_size)(VALUE self, VALUE rb_size) {
    sfVector2u size = vec2u_from_rb(rb_size);

    WB_FN(setMinimumSize)(WB_HANDLE(self), &size);

    return rb_size;
}

/* call-seq: maximum_size=(value) -> value */
static VALUE WB_METHOD(set_maximum_size)(VALUE self, VALUE rb_size) {
    sfVector2u size = vec2u_from_rb(rb_size);

    WB_FN(setMaximumSize)(WB_HANDLE(self), &size);

    return rb_size;
}

/* Through the UTF-32 entry point: the narrow setTitle decodes the bytes with
   the C locale and mangles anything outside ASCII. */
/* call-seq: title=(value) -> self */
static VALUE WB_METHOD(set_title)(VALUE self, VALUE rb_title) {
    VALUE buffer = utf32_from_rb(rb_title);

    WB_FN(setUnicodeTitle)(WB_HANDLE(self), UTF32_PTR(buffer));

    RB_GC_GUARD(buffer);

    return self;
}

/* call-seq: set_icon(size, pixels) -> pixels */
static VALUE WB_METHOD(set_icon)(VALUE self, VALUE rb_size, VALUE rb_pixels) {
    sfVector2u size = vec2u_from_rb(rb_size);
    size_t expected = (size_t)size.x * size.y * 4;

    StringValue(rb_pixels);

    if ((size_t)RSTRING_LEN(rb_pixels) < expected) {
        rb_raise(rb_eArgError, "pixel data too short: expected %zu bytes", expected);
    }

    WB_FN(setIcon)(WB_HANDLE(self), size, (const uint8_t*)RSTRING_PTR(rb_pixels));

    return rb_pixels;
}

/* call-seq: visible=(value) -> self */
static VALUE WB_METHOD(set_visible)(VALUE self, VALUE rb_visible) {
    WB_FN(setVisible)(WB_HANDLE(self), RTEST(rb_visible));
    return self;
}

/* call-seq: cursor_visible=(value) -> self */
static VALUE WB_METHOD(set_mouse_cursor_visible)(VALUE self, VALUE rb_visible) {
    WB_FN(setMouseCursorVisible)(WB_HANDLE(self), RTEST(rb_visible));
    return self;
}

/* call-seq: cursor_grabbed=(value) -> self */
static VALUE WB_METHOD(set_mouse_cursor_grabbed)(VALUE self, VALUE rb_grabbed) {
    WB_FN(setMouseCursorGrabbed)(WB_HANDLE(self), RTEST(rb_grabbed));
    return self;
}

/* CSFML has no way to clear the cursor back to a system default once set (its C
   API unconditionally dereferences the pointer, so NULL crashes rather than
   clearing), so unlike most assignment-style methods here, nil is not
   accepted. */
/* call-seq: cursor=(value) -> value */
static VALUE WB_METHOD(set_mouse_cursor)(VALUE self, VALUE rb_cursor) {
    Window* window = Get_Window_Data(self);

    if (!rb_obj_is_kind_of(rb_cursor, Get_Klass_Cursor())) {
        raise_invalid_argument_class(Get_Klass_Cursor());
    }

    WB_FN(setMouseCursor)(WB_HANDLE(self), Get_Cursor_Struct(rb_cursor));
    /* Keep the Cursor alive for as long as the window may use it. */
    window->rb_cursor = rb_cursor;

    return rb_cursor;
}

/* call-seq: key_repeat_enabled=(value) -> self */
static VALUE WB_METHOD(set_key_repeat_enabled)(VALUE self, VALUE rb_enabled) {
    WB_FN(setKeyRepeatEnabled)(WB_HANDLE(self), RTEST(rb_enabled));
    return self;
}

/* call-seq: joystick_threshold=(value) -> value */
static VALUE WB_METHOD(set_joystick_threshold)(VALUE self, VALUE rb_threshold) {
    WB_FN(setJoystickThreshold)(WB_HANDLE(self), NUM2DBL(rb_threshold));
    return rb_threshold;
}

/* call-seq: request_focus -> self */
static VALUE WB_METHOD(request_focus)(VALUE self) {
    WB_FN(requestFocus)(WB_HANDLE(self));
    return self;
}

/* call-seq: focus? -> true or false */
static VALUE WB_METHOD(has_focus)(VALUE self) {
    return BOOL2RB(WB_FN(hasFocus)(WB_HANDLE(self)));
}

/* call-seq: native_handle -> Integer */
static VALUE WB_METHOD(get_native_handle)(VALUE self) {
    return ULL2NUM((unsigned long long)(uintptr_t)WB_FN(getNativeHandle)(WB_HANDLE(self)));
}

/* Vulkan handles cross this binding as Integers, the same way
   SFML::Vulkan.function already returns one -- there is no Vulkan object model
   here to wrap them in. Returns the new VkSurfaceKHR, or nil on failure. */
/* call-seq: create_vulkan_surface(instance, allocator = nil) -> Integer or nil */
static VALUE WB_METHOD(create_vulkan_surface)(int argc, VALUE* argv, VALUE self) {
    VALUE rb_instance, rb_allocator;
    VkInstance instance;
    VkSurfaceKHR surface;
    const VkAllocationCallbacks* allocator = NULL;

    rb_scan_args(argc, argv, "11", &rb_instance, &rb_allocator);

    instance = (VkInstance)(uintptr_t)NUM2ULL(rb_instance);

    if (!NIL_P(rb_allocator)) {
        allocator = (const VkAllocationCallbacks*)(uintptr_t)NUM2ULL(rb_allocator);
    }

    if (!WB_FN(createVulkanSurface)(WB_HANDLE(self), &instance, &surface, allocator)) {
        return Qnil;
    }

    return ULL2NUM((unsigned long long)(uintptr_t)surface);
}

static void WB_METHOD(define_methods)(VALUE klass) {
    rb_define_method(klass, "is_open?", WB_METHOD(is_open), 0);
    rb_define_method(klass, "close!", WB_METHOD(close), 0);
    rb_define_method(klass, "poll_event!", WB_METHOD(poll_event), 1);
    rb_define_method(klass, "wait_event!", WB_METHOD(wait_event), 1);

    rb_define_method(klass, "position", WB_METHOD(get_position), 0);
    rb_define_method(klass, "size", WB_METHOD(get_size), 0);
    rb_define_method(klass, "focus?", WB_METHOD(has_focus), 0);
    rb_define_method(klass, "native_handle", WB_METHOD(get_native_handle), 0);

    rb_define_method(klass, "position=", WB_METHOD(set_position), 1);
    rb_define_method(klass, "size=", WB_METHOD(set_size), 1);
    rb_define_method(klass, "minimum_size=", WB_METHOD(set_minimum_size), 1);
    rb_define_method(klass, "maximum_size=", WB_METHOD(set_maximum_size), 1);
    rb_define_method(klass, "title=", WB_METHOD(set_title), 1);
    rb_define_method(klass, "set_icon", WB_METHOD(set_icon), 2);
    rb_define_method(klass, "visible=", WB_METHOD(set_visible), 1);
    rb_define_method(klass, "cursor_visible=", WB_METHOD(set_mouse_cursor_visible), 1);
    rb_define_method(klass, "cursor_grabbed=", WB_METHOD(set_mouse_cursor_grabbed), 1);
    rb_define_method(klass, "mouse_cursor=", WB_METHOD(set_mouse_cursor), 1);
    rb_define_method(klass, "cursor=", WB_METHOD(set_mouse_cursor), 1);
    rb_define_method(klass, "key_repeat_enabled=", WB_METHOD(set_key_repeat_enabled), 1);
    rb_define_method(klass, "joystick_threshold=", WB_METHOD(set_joystick_threshold), 1);

    rb_define_method(klass, "request_focus", WB_METHOD(request_focus), 0);
    rb_define_method(klass, "create_vulkan_surface", WB_METHOD(create_vulkan_surface), -1);
}

#include "window/window.h"

#include <stdio.h>

#include "graphics/render_state.h"
#include "graphics/target.h"
#include "graphics/view.h"
#include "window/video_mode.h"
#include "window/event.h"
#include "window/input_enums.h"
#include "window/context_settings.h"
#include "window/cursor.h"
#include "graphics/color.h"
#include "core/exceptions.h"
#include "system/vec2.h"
#include "core/macros.h"
#include "core/unicode.h"
#include "core/sfml.h"

static VALUE rb_cWindow;

static sfRenderWindow* Window_create(sfVideoMode* mode, const sfChar32* title, uint32_t style,
                                     sfWindowState state, const sfContextSettings* settings) {
    return sfRenderWindow_createUnicode(*mode, title, style, state, settings);
}

static void Window_free(void* ptr) {
    sfRenderWindow_destroy((sfRenderWindow*)ptr);
}

static const rb_data_type_t Window_data_type = {
    .wrap_struct_name = "SFML::Window",
    .function = {.dmark = NULL, .dfree = Window_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Window_new(int argc, VALUE* argv, VALUE klass) {
    VALUE self, rb_video_mode, rb_title, rb_style, rb_state, rb_settings, title_buffer;
    sfWindowState state = sfWindowed;
    sfContextSettings settings;
    const sfContextSettings* settings_ptr = NULL;
    uint32_t style = sfDefaultStyle;
    sfRenderWindow* window;

    rb_scan_args(argc, argv, "23", &rb_video_mode, &rb_title, &rb_style, &rb_state, &rb_settings);

    if (!rb_obj_is_kind_of(rb_video_mode, Get_Klass_Mode())) {
        raise_invalid_argument_class(Get_Klass_Mode());
    }

    if (!NIL_P(rb_style)) {
        style = window_style_from_rb(rb_style);
    }

    if (!NIL_P(rb_state)) {
        state = window_state_from_rb(rb_state);
    }

    if (!NIL_P(rb_settings)) {
        settings = context_settings_from_rb(rb_settings);
        settings_ptr = &settings;
    }

    title_buffer = utf32_from_rb(rb_title);
    window = Window_create(Get_Mode_Struct(rb_video_mode), UTF32_PTR(title_buffer), style, state,
                           settings_ptr);
    RB_GC_GUARD(title_buffer);

    if (window == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create window");
    }

    self = TypedData_Wrap_Struct(klass, &Window_data_type, window);

    rb_obj_call_init(self, argc, argv);

    return self;
}

/* Adopts an existing OS window by its native handle -- the Integer that
   Window#native_handle returns, or one obtained from a GUI toolkit. The window
   is not owned by the toolkit afterwards: destroying it stays the toolkit's
   job, and closing the Ruby object only tears down the render context. */
static VALUE Window_s_from_handle(int argc, VALUE* argv, VALUE klass) {
    VALUE self, rb_handle, rb_settings;
    sfContextSettings settings;
    const sfContextSettings* settings_ptr = NULL;
    sfRenderWindow* window;

    rb_scan_args(argc, argv, "11", &rb_handle, &rb_settings);

    if (!NIL_P(rb_settings)) {
        settings = context_settings_from_rb(rb_settings);
        settings_ptr = &settings;
    }

    window = sfRenderWindow_createFromHandle((sfWindowHandle)(uintptr_t)NUM2ULL(rb_handle),
                                             settings_ptr);

    if (window == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create window from handle");
    }

    self = TypedData_Wrap_Struct(klass, &Window_data_type, window);

    rb_obj_call_init(self, 0, NULL);

    return self;
}

static VALUE Window_init(int argc, VALUE* argv, VALUE self) {
    return self;
}

static VALUE Window_is_open(VALUE self) {
    return BOOL2RB(sfRenderWindow_isOpen(Get_Window_Struct(self)));
}

static VALUE Window_close(VALUE self) {
    sfRenderWindow_close(Get_Window_Struct(self));
    return self;
}

static VALUE Window_clear(int argc, VALUE* argv, VALUE self) {
    sfColor color = sfBlack;

    if (argc > 1) {
        raise_invalid_arguments_excepted(1, argc);
    }

    if (argc == 1) {
        color_check(argv[0]);
        color = color_new_from_rb(argv[0]);
    }

    sfRenderWindow_clear(Get_Window_Struct(self), color);

    return self;
}

static VALUE Window_display(VALUE self) {
    sfRenderWindow_display(Get_Window_Struct(self));
    return self;
}

static VALUE Window_poll_event(VALUE self, VALUE rb_event) {
    if (!rb_obj_is_kind_of(rb_event, Get_Klass_Event())) {
        raise_invalid_argument_class(Get_Klass_Event());
    }

    return BOOL2RB(sfRenderWindow_pollEvent(Get_Window_Struct(self), Get_Event_Struct(rb_event)));
}

static VALUE Window_wait_event(VALUE self, VALUE rb_event) {
    if (!rb_obj_is_kind_of(rb_event, Get_Klass_Event())) {
        raise_invalid_argument_class(Get_Klass_Event());
    }

    return BOOL2RB(
        sfRenderWindow_waitEvent(Get_Window_Struct(self), sfTime_Zero, Get_Event_Struct(rb_event)));
}

static VALUE Window_get_position(VALUE self) {
    return VEC2_C2RB(sfRenderWindow_getPosition(Get_Window_Struct(self)));
}

static VALUE Window_set_position(VALUE self, VALUE rb_arr) {
    sfRenderWindow_setPosition(Get_Window_Struct(self), vec2i_new_from_ruby(rb_arr));
    return self;
}

static VALUE Window_set_frame_rate(VALUE self, VALUE rb_limit) {
    sfRenderWindow_setFramerateLimit(Get_Window_Struct(self), NUM2INT(rb_limit));
    return self;
}

static VALUE Window_set_size(VALUE self, VALUE rb_size) {
    sfRenderWindow_setSize(Get_Window_Struct(self), vec2u_from_rb(rb_size));
    return self;
}

static VALUE Window_get_size(VALUE self) {
    return VEC2_C2RB(sfRenderWindow_getSize(Get_Window_Struct(self)));
}

static VALUE Window_set_minimum_size(VALUE self, VALUE rb_size) {
    sfVector2u size = vec2u_from_rb(rb_size);

    sfRenderWindow_setMinimumSize(Get_Window_Struct(self), &size);

    return rb_size;
}

static VALUE Window_set_maximum_size(VALUE self, VALUE rb_size) {
    sfVector2u size = vec2u_from_rb(rb_size);

    sfRenderWindow_setMaximumSize(Get_Window_Struct(self), &size);

    return rb_size;
}

static VALUE Window_set_active(VALUE self, VALUE rb_active) {
    return BOOL2RB(sfRenderWindow_setActive(Get_Window_Struct(self), RTEST(rb_active)));
}

static VALUE Window_get_native_handle(VALUE self) {
    return ULL2NUM(
        (unsigned long long)(uintptr_t)sfRenderWindow_getNativeHandle(Get_Window_Struct(self)));
}

/* Vulkan handles cross this binding as Integers, the same way
   SFML::Vulkan.function already returns one -- there is no Vulkan object model
   here to wrap them in. Returns the new VkSurfaceKHR, or nil if creation
   failed. */
static VALUE Window_create_vulkan_surface(int argc, VALUE* argv, VALUE self) {
    VALUE rb_instance, rb_allocator;
    VkInstance instance;
    VkSurfaceKHR surface;
    const VkAllocationCallbacks* allocator = NULL;

    rb_scan_args(argc, argv, "11", &rb_instance, &rb_allocator);

    instance = (VkInstance)(uintptr_t)NUM2ULL(rb_instance);

    if (!NIL_P(rb_allocator)) {
        allocator = (const VkAllocationCallbacks*)(uintptr_t)NUM2ULL(rb_allocator);
    }

    if (!sfRenderWindow_createVulkanSurface(Get_Window_Struct(self), &instance, &surface,
                                            allocator)) {
        return Qnil;
    }

    return ULL2NUM((unsigned long long)(uintptr_t)surface);
}

static VALUE Window_set_icon(VALUE self, VALUE rb_size, VALUE rb_pixels) {
    StringValue(rb_pixels);

    sfRenderWindow_setIcon(Get_Window_Struct(self), vec2u_from_rb(rb_size),
                           (const uint8_t*)RSTRING_PTR(rb_pixels));

    return rb_pixels;
}

static VALUE Window_get_settings(VALUE self) {
    return context_settings_to_rb(sfRenderWindow_getSettings(Get_Window_Struct(self)));
}

/* Through the UTF-32 entry point: sfRenderWindow_setTitle decodes the bytes
   with the C locale and mangles anything outside ASCII. */
static VALUE Window_set_title(VALUE self, VALUE rb_title) {
    VALUE buffer = utf32_from_rb(rb_title);

    sfRenderWindow_setUnicodeTitle(Get_Window_Struct(self), UTF32_PTR(buffer));

    RB_GC_GUARD(buffer);

    return self;
}

static VALUE Window_set_visible(VALUE self, VALUE rb_visible) {
    sfRenderWindow_setVisible(Get_Window_Struct(self), RTEST(rb_visible));
    return self;
}

static VALUE Window_set_vertical_sync_enabled(VALUE self, VALUE rb_enable) {
    sfRenderWindow_setVerticalSyncEnabled(Get_Window_Struct(self), RTEST(rb_enable));
    return self;
}

static VALUE Window_set_mouse_cursor_visible(VALUE self, VALUE rb_visible) {
    sfRenderWindow_setMouseCursorVisible(Get_Window_Struct(self), RTEST(rb_visible));
    return self;
}

static VALUE Window_set_mouse_cursor_grabbed(VALUE self, VALUE rb_grabbed) {
    sfRenderWindow_setMouseCursorGrabbed(Get_Window_Struct(self), RTEST(rb_grabbed));
    return self;
}

static VALUE Window_set_mouse_cursor(VALUE self, VALUE rb_cursor) {
    if (NIL_P(rb_cursor)) {
        sfRenderWindow_setMouseCursor(Get_Window_Struct(self), NULL);
        return rb_cursor;
    }

    if (!rb_obj_is_kind_of(rb_cursor, Get_Klass_Cursor())) {
        raise_invalid_argument_class(Get_Klass_Cursor());
    }

    sfRenderWindow_setMouseCursor(Get_Window_Struct(self), Get_Cursor_Struct(rb_cursor));

    return rb_cursor;
}

static VALUE Window_set_key_repeat_enabled(VALUE self, VALUE rb_enabled) {
    sfRenderWindow_setKeyRepeatEnabled(Get_Window_Struct(self), RTEST(rb_enabled));
    return self;
}

static VALUE Window_set_joystick_threshold(VALUE self, VALUE rb_threshold) {
    sfRenderWindow_setJoystickThreshold(Get_Window_Struct(self), NUM2DBL(rb_threshold));
    return rb_threshold;
}

static VALUE Window_request_focus(VALUE self) {
    sfRenderWindow_requestFocus(Get_Window_Struct(self));
    return self;
}

static VALUE Window_has_focus(VALUE self) {
    return BOOL2RB(sfRenderWindow_hasFocus(Get_Window_Struct(self)));
}

static VALUE Window_draw(int argc, VALUE* argv, VALUE self) {
    VALUE rb_drawable, rb_state;

    if (argc == 0 || argc > 2) {
        raise_invalid_arguments_excepted(-1, argc);
    }

    rb_drawable = argv[0];
    rb_state = (argc == 2) ? argv[1] : Get_New_RenderState();

    rb_funcall(Get_New_Target(self), rb_intern("draw"), 2, rb_drawable, rb_state);

    return self;
}

static VALUE Window_set_view(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        rb_raise(rb_eArgError, "invalid object, expected a View object");
    }

    sfRenderWindow_setView(Get_Window_Struct(self), Get_View_Struct(rb_view));

    return self;
}

static VALUE Window_get_view(VALUE self) {
    return Get_Casting_View(sfView_copy(sfRenderWindow_getView(Get_Window_Struct(self))));
}

static VALUE Window_get_default_view(VALUE self) {
    return Get_Casting_View(sfView_copy((sfRenderWindow_getDefaultView(Get_Window_Struct(self)))));
}

#define RT_FN(name) sfRenderWindow_##name
#define RT_METHOD(name) Window_##name
#define RT_HANDLE(self) Get_Window_Struct(self)
#include "graphics/render_target.inc"
#undef RT_FN
#undef RT_METHOD
#undef RT_HANDLE

void Init_Window(VALUE rb_module) {
    rb_cWindow = rb_define_class_under(rb_module, "Window", rb_cObject);

    rb_define_singleton_method(rb_cWindow, "new", Window_new, -1);
    rb_define_singleton_method(rb_cWindow, "from_handle", Window_s_from_handle, -1);

    // methods
    rb_define_method(rb_cWindow, "initialize", Window_init, -1);
    rb_define_method(rb_cWindow, "is_open?", Window_is_open, 0);
    rb_define_method(rb_cWindow, "close!", Window_close, 0);
    rb_define_method(rb_cWindow, "clear", Window_clear, -1);
    rb_define_method(rb_cWindow, "display", Window_display, 0);
    rb_define_method(rb_cWindow, "poll_event!", Window_poll_event, 1);
    rb_define_method(rb_cWindow, "wait_event!", Window_wait_event, 1);

    // setters
    rb_define_method(rb_cWindow, "frame_rate=", Window_set_frame_rate, 1);
    rb_define_method(rb_cWindow, "framerate_limit=", Window_set_frame_rate, 1);
    rb_define_method(rb_cWindow, "size=", Window_set_size, 1);
    rb_define_method(rb_cWindow, "minimum_size=", Window_set_minimum_size, 1);
    rb_define_method(rb_cWindow, "maximum_size=", Window_set_maximum_size, 1);
    rb_define_method(rb_cWindow, "title=", Window_set_title, 1);
    rb_define_method(rb_cWindow, "set_icon", Window_set_icon, 2);
    rb_define_method(rb_cWindow, "visible=", Window_set_visible, 1);
    rb_define_method(rb_cWindow, "active=", Window_set_active, 1);
    rb_define_method(rb_cWindow, "vertical_sync_enabled=", Window_set_vertical_sync_enabled, 1);
    rb_define_method(rb_cWindow, "cursor_visible=", Window_set_mouse_cursor_visible, 1);
    rb_define_method(rb_cWindow, "cursor_grabbed=", Window_set_mouse_cursor_grabbed, 1);
    rb_define_method(rb_cWindow, "mouse_cursor=", Window_set_mouse_cursor, 1);
    rb_define_method(rb_cWindow, "cursor=", Window_set_mouse_cursor, 1);
    rb_define_method(rb_cWindow, "key_repeat_enabled=", Window_set_key_repeat_enabled, 1);
    rb_define_method(rb_cWindow, "joystick_threshold=", Window_set_joystick_threshold, 1);
    rb_define_method(rb_cWindow, "position=", Window_set_position, 1);
    rb_define_method(rb_cWindow, "view=", Window_set_view, 1);

    // getters
    rb_define_method(rb_cWindow, "request_focus", Window_request_focus, 0);
    rb_define_method(rb_cWindow, "focus?", Window_has_focus, 0);
    rb_define_method(rb_cWindow, "draw", Window_draw, -1);
    rb_define_method(rb_cWindow, "position", Window_get_position, 0);
    rb_define_method(rb_cWindow, "size", Window_get_size, 0);
    rb_define_method(rb_cWindow, "view", Window_get_view, 0);
    rb_define_method(rb_cWindow, "default_view", Window_get_default_view, 0);
    rb_define_method(rb_cWindow, "settings", Window_get_settings, 0);
    rb_define_method(rb_cWindow, "native_handle", Window_get_native_handle, 0);
    rb_define_method(rb_cWindow, "create_vulkan_surface", Window_create_vulkan_surface, -1);

    Window_define_render_target_methods(rb_cWindow);
}

void* Get_Window_Struct(VALUE self) {
    sfRenderWindow* ptr;
    TypedData_Get_Struct(self, sfRenderWindow, &Window_data_type, ptr);
    return ptr;
}

VALUE Get_Klass_Window() {
    return rb_cWindow;
}
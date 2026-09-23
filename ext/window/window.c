#include "window/window.h"

#include <stdio.h>

#include "graphics/render_state.h"
#include "graphics/target.h"
#include "graphics/view.h"
#include "window/window_base.h"
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

/* Window overrides every WindowBase method with the sfRenderWindow_ entry
   point: its handle is an sfRenderWindow*, and CSFML keys its functions off the
   concrete type, so inheriting the sfWindowBase_ bodies would be wrong. The
   generated bodies live in window_base.inc. */
#define WB_FN(name) sfRenderWindow_##name
#define WB_METHOD(name) Window_##name
#define WB_HANDLE(self) Get_Window_Struct(self)
#include "window/window_base.inc"
#undef WB_FN
#undef WB_METHOD
#undef WB_HANDLE

/* call-seq:
 *   Window.new(video_mode, title, style = :default, state = :windowed, settings = nil) -> Window
 *
 * Creates a window with an OpenGL context from +video_mode+ and +title+.
 *
 * @return [Window]
 * @raise [RuntimeError] if window creation fails
 */
static VALUE Window_initialize(int argc, VALUE* argv, VALUE self) {
    VALUE rb_video_mode, rb_title, rb_style, rb_state, rb_settings, title_buffer;
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
    window = sfRenderWindow_createUnicode(*Get_Mode_Struct(rb_video_mode), UTF32_PTR(title_buffer),
                                          style, state, settings_ptr);
    RB_GC_GUARD(title_buffer);

    if (window == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create window");
    }

    Get_Window_Data(self)->handle = window;
    Get_Window_Data(self)->kind = SFML_WINDOW_KIND_WINDOW;

    return self;
}

/* call-seq:
 *   Window.from_handle(handle, settings = nil) -> Window
 *
 * Adopts an existing OS window by its native handle -- the Integer that
 * Window#native_handle returns, or one obtained from a GUI toolkit. The window
 * is not owned by the toolkit afterwards: destroying it stays the toolkit's
 * job, and closing the Ruby object only tears down the render context.
 *
 * @return [Window]
 * @raise [RuntimeError] if window creation fails
 */
static VALUE Window_s_from_handle(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_handle, rb_settings;
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

    return Window_wrap_handle(klass, window, SFML_WINDOW_KIND_WINDOW);
}

/* call-seq:
 *   clear(color = Color::BLACK) -> self
 *
 * Clears the window to +color+ (black by default).
 *
 * @return [self]
 * @raise [ArgumentError] if given more than one argument
 */
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

/* call-seq: display -> self
 *
 * Presents everything drawn since the last call to the screen.
 *
 * @return [self]
 */
static VALUE Window_display(VALUE self) {
    sfRenderWindow_display(Get_Window_Struct(self));
    return self;
}

/* call-seq:
 *   frame_rate=(value) -> self
 *
 * Limits the framerate to +value+ frames per second; 0 disables the limit.
 *
 * @return [self]
 */
static VALUE Window_set_frame_rate(VALUE self, VALUE rb_limit) {
    sfRenderWindow_setFramerateLimit(Get_Window_Struct(self), NUM2INT(rb_limit));
    return self;
}

/* call-seq:
 *   active=(value) -> true or false
 *
 * Activates or deactivates this window's OpenGL context as the current one on
 * the calling thread.
 *
 * @return [Boolean] whether activation succeeded
 */
static VALUE Window_set_active(VALUE self, VALUE rb_active) {
    return BOOL2RB(sfRenderWindow_setActive(Get_Window_Struct(self), RTEST(rb_active)));
}

/* call-seq:
 *   vertical_sync_enabled=(value) -> self
 *
 * Enables or disables vertical synchronization.
 *
 * @return [self]
 */
static VALUE Window_set_vertical_sync_enabled(VALUE self, VALUE rb_enable) {
    sfRenderWindow_setVerticalSyncEnabled(Get_Window_Struct(self), RTEST(rb_enable));
    return self;
}

/* call-seq: settings -> ContextSettings
 *
 * Returns the context settings this window was created with.
 *
 * @return [ContextSettings] the settings this window's context was created with
 */
static VALUE Window_get_settings(VALUE self) {
    return context_settings_to_rb(sfRenderWindow_getSettings(Get_Window_Struct(self)));
}

/* call-seq:
 *   draw(drawable, state = nil) -> self
 *
 * Draws +drawable+ into the window.
 *
 * @return [self]
 * @raise [ArgumentError] if given no arguments or more than 2
 */
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

/* call-seq: view=(value) -> self
 *
 * Sets the window's active view to +value+.
 *
 * @return [self]
 * @raise [ArgumentError] if +value+ is not a View
 */
static VALUE Window_set_view(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        rb_raise(rb_eArgError, "invalid object, expected a View object");
    }

    sfRenderWindow_setView(Get_Window_Struct(self), Get_View_Struct(rb_view));

    return self;
}

/* call-seq: view -> View
 *
 * Returns a copy of the window's currently active view.
 *
 * @return [View] a copy of the window's currently active view
 */
static VALUE Window_get_view(VALUE self) {
    return Get_Casting_View(sfView_copy(sfRenderWindow_getView(Get_Window_Struct(self))));
}

/* call-seq: default_view -> View
 *
 * Returns a copy of the window's default view.
 *
 * @return [View] a copy of the window's default view
 */
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

/* Document-class: SF::Window::Window
 * A renderable OS window with an OpenGL context attached. It derives from
 * SF::Window::WindowBase; SF::Graphics::RenderWindow is the same object under the name that
 * includes SF::Graphics::RenderTarget.
 *
 * @!method open?
 *   Returns +true+ while the window is open. +is_open?+ is a deprecated alias.
 *   @return [Boolean]
 * @!method close!
 *   Closes the window.
 *   @return [self]
 * @!method poll_event!(event)
 *   Pops the next pending event into +event+, if any, without blocking.
 *   @return [Boolean] whether an event was popped
 * @!method poll_events! { |event| ... }
 *   Pops every pending event and yields it; returns an Enumerator without a block.
 *   @return [self, Enumerator]
 * @!method wait_event!(event)
 *   Blocks until an event is available and pops it into +event+.
 *   @return [Boolean] whether an event was popped
 * @!method position
 *   Returns the window's position in desktop coordinates.
 *   @return [Vector2]
 * @!method position=(value)
 *   Moves the window to +value+.
 *   @return [Vector2] +value+
 * @!method size
 *   Returns the client area size in pixels.
 *   @return [Vector2]
 * @!method size=(value)
 *   Resizes the window's client area.
 *   @return [self]
 * @!method minimum_size=(value)
 *   Sets the window's minimum allowed client size.
 *   @return [Vector2] +value+
 * @!method maximum_size=(value)
 *   Sets the window's maximum allowed client size.
 *   @return [Vector2] +value+
 * @!method title=(value)
 *   Sets the window title.
 *   @return [self]
 * @!method set_icon(size, pixels)
 *   Sets the window's icon from RGBA32 pixel data.
 *   @return [String] +pixels+
 * @!method visible=(value)
 *   Shows or hides the window.
 *   @return [self]
 * @!method visible?
 *   Returns the last value passed to #visible=, since CSFML 3 has no window
 *   visibility getter.
 *   @return [Boolean]
 * @!method clear!(color = Color::BLACK)
 *   Clears the window to +color+. +clear+ is a deprecated alias.
 *   @return [self]
 * @!method display!
 *   Presents everything drawn since the last clear. +display+ is a deprecated alias.
 *   @return [self]
 * @!method render!(clear_color: Color::BLACK) { |window| ... }
 *   Clears, yields the window for drawing, then presents.
 *   @return [self]
 * @!method cursor_visible=(value)
 *   Shows or hides the mouse cursor over the window.
 *   @return [self]
 * @!method cursor_grabbed=(value)
 *   Confines or releases the mouse cursor to the window's client area.
 *   @return [self]
 * @!method mouse_cursor=(value)
 *   Sets the window's mouse cursor.
 *   @return [Cursor] +value+
 * @!method cursor=(value)
 *   Sets the window's mouse cursor.
 *   @return [Cursor] +value+
 * @!method key_repeat_enabled=(value)
 *   Enables or disables key-repeat events.
 *   @return [self]
 * @!method joystick_threshold=(value)
 *   Sets the minimum joystick axis change that generates a move event.
 *   @return [Float] +value+
 * @!method request_focus!
 *   Requests focus for this window. +request_focus+ is a deprecated alias.
 *   @return [self]
 * @!method focused?
 *   Returns +true+ if the window currently has focus. +focus?+ is a deprecated alias.
 *   @return [Boolean]
 * @!method native_handle
 *   Returns the OS-specific window handle.
 *   @return [Integer]
 * @!method create_vulkan_surface(instance, allocator = nil)
 *   Creates a Vulkan surface for this window.
 *   @return [Integer, nil] the new +VkSurfaceKHR+, or +nil+ if creation failed
 * @!method srgb?
 *   Returns +true+ if the target's framebuffer is sRGB-capable.
 *   @return [Boolean]
 * @!method clear_stencil(value)
 *   Clears the stencil buffer to +value+.
 *   @return [self]
 * @!method clear_color_and_stencil(color, stencil)
 *   Clears the color buffer to +color+ and the stencil buffer to +stencil+.
 *   @return [self]
 * @!method viewport(view = nil)
 *   Returns the current viewport rectangle in pixels.
 *   @return [Rect] the current viewport in pixels; +view+ defaults to the target's current view
 * @!method scissor(view = nil)
 *   Returns the current scissor rectangle in pixels.
 *   @return [Rect] the current scissor rectangle in pixels; +view+ defaults to the target's current
 * view
 * @!method map_pixel_to_coords(point, view = nil)
 *   Converts a pixel position to world coordinates, using the inverse of the view transform.
 *   @return [Vector2]
 * @!method map_coords_to_pixel(point, view = nil)
 *   Converts a world position to pixel coordinates.
 *   @return [Vector2]
 * @!method push_gl_states
 *   Saves the current OpenGL state.
 *   @return [self]
 * @!method pop_gl_states
 *   Restores the OpenGL state saved by #push_gl_states.
 *   @return [self]
 * @!method reset_gl_states
 *   Resets the OpenGL state to SFML's defaults.
 *   @return [self]
 * @!method draw_primitives(vertices, primitive, state = nil)
 *   Draws raw vertex data as the given primitive type.
 *   @return [self]
 * @!method draw_vertex_buffer_range(buffer, first, count, state = nil)
 *   Draws a range of vertices from a vertex buffer.
 *   @return [self]
 */
void Init_Window(VALUE rb_mWindow) {
    rb_cWindow = rb_define_class_under(rb_mWindow, "Window", Get_Klass_WindowBase());

    rb_define_alloc_func(rb_cWindow, Window_alloc);

    rb_define_singleton_method(rb_cWindow, "from_handle", Window_s_from_handle, -1);

    // methods generated from window_base.inc, overridden with sfRenderWindow_*
    Window_define_methods(rb_cWindow);

    // methods
    rb_define_method(rb_cWindow, "initialize", Window_initialize, -1);
    rb_define_method(rb_cWindow, "clear", Window_clear, -1);
    rb_define_method(rb_cWindow, "display", Window_display, 0);

    // setters
    rb_define_method(rb_cWindow, "frame_rate=", Window_set_frame_rate, 1);
    rb_define_method(rb_cWindow, "framerate_limit=", Window_set_frame_rate, 1);
    rb_define_method(rb_cWindow, "active=", Window_set_active, 1);
    rb_define_method(rb_cWindow, "vertical_sync_enabled=", Window_set_vertical_sync_enabled, 1);
    rb_define_method(rb_cWindow, "view=", Window_set_view, 1);

    // getters
    rb_define_method(rb_cWindow, "draw", Window_draw, -1);
    rb_define_method(rb_cWindow, "view", Window_get_view, 0);
    rb_define_method(rb_cWindow, "default_view", Window_get_default_view, 0);
    rb_define_method(rb_cWindow, "settings", Window_get_settings, 0);

    Window_define_render_target_methods(rb_cWindow);
}

VALUE Get_Klass_Window(void) {
    return rb_cWindow;
}

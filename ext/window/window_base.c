#include "window/window_base.h"

#include <ruby.h>

#include "window/video_mode.h"
#include "window/input_enums.h"
#include "core/exceptions.h"
#include "core/macros.h"
#include "core/unicode.h"

static VALUE rb_cWindowBase;

static void Window_mark(void* ptr) {
    Window* window = ptr;

    rb_gc_mark(window->rb_cursor);
}

static void Window_free(void* ptr) {
    Window* window = ptr;

    if (window->handle != NULL) {
        if (window->kind == SFML_WINDOW_KIND_WINDOW) {
            sfRenderWindow_destroy(window->handle);
        } else {
            sfWindowBase_destroy(window->handle);
        }
    }

    free(window);
}

static const rb_data_type_t Window_data_type = {
    .wrap_struct_name = "SFML::Window",
    .function = {.dmark = Window_mark, .dfree = Window_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

VALUE Window_alloc(VALUE klass) {
    Window* window = malloc(sizeof(Window));

    if (window == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate window");
    }

    window->handle = NULL;
    window->kind = SFML_WINDOW_KIND_BASE;
    window->rb_cursor = Qnil;

    return TypedData_Wrap_Struct(klass, &Window_data_type, window);
}

VALUE Window_wrap_handle(VALUE klass, void* handle, WindowKind kind) {
    Window* window = malloc(sizeof(Window));

    if (window == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate window");
    }

    window->handle = handle;
    window->kind = kind;
    window->rb_cursor = Qnil;

    return TypedData_Wrap_Struct(klass, &Window_data_type, window);
}

/* call-seq: initialize_copy(other) -> self
 *
 * Copy construction is not supported: a WindowBase wraps a native window that
 * cannot be duplicated, so this always raises.
 *
 * @raise [TypeError] always
 */
static VALUE Window_initialize_copy(VALUE self, VALUE other) {
    (void)other;
    rb_raise(rb_eTypeError, "can't copy a %s", rb_obj_classname(self));
}

/* call-seq:
 *   WindowBase.new(video_mode, title, style = :default, state = :windowed) -> WindowBase
 *
 * Creates an OS window with no OpenGL context of its own.
 *
 * @return [WindowBase]
 * @raise [RuntimeError] if window creation fails
 */
static VALUE WindowBase_initialize(int argc, VALUE* argv, VALUE self) {
    VALUE rb_video_mode, rb_title, rb_style, rb_state, title_buffer;
    sfWindowState state = sfWindowed;
    uint32_t style = sfDefaultStyle;
    sfWindowBase* window;

    rb_scan_args(argc, argv, "22", &rb_video_mode, &rb_title, &rb_style, &rb_state);

    if (!rb_obj_is_kind_of(rb_video_mode, Get_Klass_Mode())) {
        raise_invalid_argument_class(Get_Klass_Mode());
    }

    if (!NIL_P(rb_style)) {
        style = window_style_from_rb(rb_style);
    }

    if (!NIL_P(rb_state)) {
        state = window_state_from_rb(rb_state);
    }

    title_buffer = utf32_from_rb(rb_title);
    window = sfWindowBase_createUnicode(*Get_Mode_Struct(rb_video_mode), UTF32_PTR(title_buffer),
                                        style, state);
    RB_GC_GUARD(title_buffer);

    if (window == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create window");
    }

    Get_Window_Data(self)->handle = window;
    Get_Window_Data(self)->kind = SFML_WINDOW_KIND_BASE;

    return self;
}

/* call-seq:
 *   WindowBase.from_handle(handle) -> WindowBase
 *
 * Adopts an existing OS window by its native handle -- the Integer that
 * WindowBase#native_handle returns, or one obtained from a GUI toolkit. The
 * window is not owned by the toolkit afterwards: destroying it stays the
 * toolkit's job, and closing the Ruby object only tears down this wrapper.
 *
 * @return [WindowBase]
 * @raise [RuntimeError] if window creation fails
 */
static VALUE WindowBase_s_from_handle(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_handle;
    sfWindowBase* window;

    rb_scan_args(argc, argv, "1", &rb_handle);

    window = sfWindowBase_createFromHandle((sfWindowHandle)(uintptr_t)NUM2ULL(rb_handle));

    if (window == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create window from handle");
    }

    return Window_wrap_handle(klass, window, SFML_WINDOW_KIND_BASE);
}

#define WB_FN(name) sfWindowBase_##name
#define WB_METHOD(name) WindowBase_##name
#define WB_HANDLE(self) Get_WindowBase_Struct(self)
#include "window/window_base.inc"
#undef WB_FN
#undef WB_METHOD
#undef WB_HANDLE

/* Document-class: SFML::WindowBase
 * An OS window and its event queue, without an OpenGL context. It is the base
 * of SFML::Window and SFML::RenderWindow, and every method below is also
 * available -- backed by the matching sfRenderWindow entry point -- on both.
 *
 * @!method self.open(video_mode, title, style = :default, state = :windowed)
 *   Creates a window, yields it and closes it when the block returns -- normally
 *   or by raising. Without a block, returns the open window for the caller to
 *   close. Rubyesque (Matz-like) over +new+ plus +close!+.
 *   @yield [window] the freshly created, open window
 *   @return [WindowBase]
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
 *   Pops every pending event and yields each one, then returns self. Without a
 *   block it returns an Enumerator. Rubyesque (Matz-like) over a +poll_event!+ loop.
 *   @yield [event] each pending event
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
 *   Returns the last value passed to #visible=, since CSFML 3 exposes no window
 *   visibility getter.
 *   @return [Boolean]
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
 *   Returns +true+ if the window currently has focus. +focus?+ is a deprecated
 *   alias.
 *   @return [Boolean]
 * @!method native_handle
 *   Returns the OS-specific window handle.
 *   @return [Integer]
 * @!method create_vulkan_surface(instance, allocator = nil)
 *   Creates a Vulkan surface for this window.
 *   @return [Integer, nil] the new +VkSurfaceKHR+, or +nil+ if creation failed
 */
void Init_WindowBase(VALUE rb_mSFML) {
    rb_cWindowBase = rb_define_class_under(rb_mSFML, "WindowBase", rb_cObject);

    rb_define_alloc_func(rb_cWindowBase, Window_alloc);

    rb_define_singleton_method(rb_cWindowBase, "from_handle", WindowBase_s_from_handle, -1);

    rb_define_method(rb_cWindowBase, "initialize", WindowBase_initialize, -1);
    rb_define_private_method(rb_cWindowBase, "initialize_copy", Window_initialize_copy, 1);

    WindowBase_define_methods(rb_cWindowBase);
}

VALUE Get_Klass_WindowBase(void) {
    return rb_cWindowBase;
}

Window* Get_Window_Data(VALUE self) {
    Window* ptr;
    TypedData_Get_Struct(self, Window, &Window_data_type, ptr);
    return ptr;
}

sfWindowBase* Get_WindowBase_Struct(VALUE self) {
    return Get_Window_Data(self)->handle;
}

sfRenderWindow* Get_Window_Struct(VALUE self) {
    return Get_Window_Data(self)->handle;
}

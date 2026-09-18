#include "graphics/render_texture.h"

#include <ruby.h>

#include "graphics/target.h"
#include "graphics/render_state.h"
#include "graphics/view.h"
#include "graphics/texture.h"
#include "graphics/color.h"
#include "graphics/rect.h"
#include "system/vec2.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cRenderTexture;

static void RenderTexture_free(void* ptr) {
    sfRenderTexture_destroy(ptr);
}

static const rb_data_type_t RenderTexture_data_type = {
    .wrap_struct_name = "SFML::RenderTexture",
    .function = {.dmark = NULL, .dfree = RenderTexture_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

sfRenderTexture* Get_RenderTexture_Struct(VALUE self) {
    sfRenderTexture* ptr;
    TypedData_Get_Struct(self, sfRenderTexture, &RenderTexture_data_type, ptr);
    return ptr;
}

static VALUE RenderTexture_wrap(VALUE klass, sfRenderTexture* render_texture) {
    if (render_texture == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create render texture");
    }

    return TypedData_Wrap_Struct(klass, &RenderTexture_data_type, render_texture);
}

/* call-seq:
 *   RenderTexture.new(size)           -> RenderTexture
 *   RenderTexture.new(size, settings) -> RenderTexture
 *
 * +settings+ is currently accepted but ignored.
 *
 * @return [RenderTexture]
 * @raise [RuntimeError] if creation fails
 */
static VALUE RenderTexture_new(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_size, rb_settings;

    rb_scan_args(argc, argv, "11", &rb_size, &rb_settings);

    (void)rb_settings;

    return RenderTexture_wrap(klass, sfRenderTexture_create(vec2u_from_rb(rb_size), NULL));
}

/* call-seq: size -> Vector2
 *
 * @return [Vector2]
 */
static VALUE RenderTexture_get_size(VALUE self) {
    sfVector2u size = sfRenderTexture_getSize(Get_RenderTexture_Struct(self));

    return vec2f_to_rb((sfVector2f){(float)size.x, (float)size.y});
}

/* call-seq:
 *   active=(value) -> true or false
 *
 * Activates or deactivates this render texture as the current OpenGL
 * rendering target on the calling thread.
 *
 * @return [Boolean] whether the operation succeeded
 */
static VALUE RenderTexture_set_active(VALUE self, VALUE rb_active) {
    return BOOL2RB(sfRenderTexture_setActive(Get_RenderTexture_Struct(self), RTEST(rb_active)));
}

/* call-seq: display -> self
 *
 * Updates the target texture with everything drawn so far. Call this after
 * drawing and before reading #texture.
 *
 * @return [self]
 */
static VALUE RenderTexture_display(VALUE self) {
    sfRenderTexture_display(Get_RenderTexture_Struct(self));
    return self;
}

/* call-seq:
 *   clear             -> self
 *   clear(color)       -> self
 *
 * Clears the render texture to +color+ (a Color, default black).
 *
 * @return [self]
 */
static VALUE RenderTexture_clear(int argc, VALUE* argv, VALUE self) {
    VALUE rb_color;
    sfColor color = sfBlack;

    rb_scan_args(argc, argv, "01", &rb_color);

    if (!NIL_P(rb_color)) {
        color = color_from_rb(rb_color);
    }

    sfRenderTexture_clear(Get_RenderTexture_Struct(self), color);

    return self;
}

/* call-seq:
 *   view=(value) -> self
 *
 * @return [self]
 * @raise [TypeError] if +value+ is not a View
 */
static VALUE RenderTexture_set_view(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        raise_invalid_argument_class(Get_Klass_View());
    }

    sfRenderTexture_setView(Get_RenderTexture_Struct(self), Get_View_Struct(rb_view));

    return self;
}

/* call-seq: view -> View
 *
 * @return [View] a copy of the current view
 */
static VALUE RenderTexture_get_view(VALUE self) {
    return Get_Casting_View(sfView_copy(sfRenderTexture_getView(Get_RenderTexture_Struct(self))));
}

/* call-seq: default_view -> View
 *
 * @return [View] a copy of the view covering the render texture's full area
 */
static VALUE RenderTexture_get_default_view(VALUE self) {
    return Get_Casting_View(
        sfView_copy(sfRenderTexture_getDefaultView(Get_RenderTexture_Struct(self))));
}

/* call-seq: texture -> Texture
 *
 * @return [Texture] the target texture, borrowed -- its contents update
 *   after each #display
 */
static VALUE RenderTexture_get_texture(VALUE self) {
    return texture_from_borrowed(sfRenderTexture_getTexture(Get_RenderTexture_Struct(self)));
}

/* call-seq:
 *   smooth=(value) -> true or false
 *
 * @return [Boolean] +value+
 */
static VALUE RenderTexture_set_smooth(VALUE self, VALUE rb_smooth) {
    sfRenderTexture_setSmooth(Get_RenderTexture_Struct(self), RTEST(rb_smooth));
    return rb_smooth;
}

/* call-seq: smooth? -> true or false
 *
 * @return [Boolean]
 */
static VALUE RenderTexture_is_smooth(VALUE self) {
    return BOOL2RB(sfRenderTexture_isSmooth(Get_RenderTexture_Struct(self)));
}

/* call-seq:
 *   repeated=(value) -> true or false
 *
 * @return [Boolean] +value+
 */
static VALUE RenderTexture_set_repeated(VALUE self, VALUE rb_repeated) {
    sfRenderTexture_setRepeated(Get_RenderTexture_Struct(self), RTEST(rb_repeated));
    return rb_repeated;
}

/* call-seq: repeated? -> true or false
 *
 * @return [Boolean]
 */
static VALUE RenderTexture_is_repeated(VALUE self) {
    return BOOL2RB(sfRenderTexture_isRepeated(Get_RenderTexture_Struct(self)));
}

/* call-seq: generate_mipmap -> true or false
 *
 * @return [Boolean] whether mipmap generation succeeded
 */
static VALUE RenderTexture_generate_mipmap(VALUE self) {
    return BOOL2RB(sfRenderTexture_generateMipmap(Get_RenderTexture_Struct(self)));
}

/* call-seq:
 *   draw(drawable)         -> self
 *   draw(drawable, state)  -> self
 *
 * Draws +drawable+ (anything responding to +#draw+, i.e. including
 * Drawable) using +state+ (a RenderState, default the identity state).
 *
 * @return [self]
 * @raise [ArgumentError] if given no arguments or more than 2
 */
static VALUE RenderTexture_draw(int argc, VALUE* argv, VALUE self) {
    VALUE rb_drawable, rb_state;

    if (argc == 0 || argc > 2) {
        raise_invalid_arguments_excepted(-1, argc);
    }

    rb_drawable = argv[0];
    rb_state = (argc == 2) ? argv[1] : Get_New_RenderState();

    rb_funcall(Get_New_Target_From_RenderTexture(self), rb_intern("draw"), 2, rb_drawable,
               rb_state);

    return self;
}

/* call-seq:
 *   RenderTexture.maximum_antialiasing_level -> Integer
 *
 * @return [Integer]
 */
static VALUE RenderTexture_maximum_antialiasing_level(VALUE klass) {
    return UINT2NUM(sfRenderTexture_getMaximumAntiAliasingLevel());
}

#define RT_FN(name) sfRenderTexture_##name
#define RT_METHOD(name) RenderTexture_##name
#define RT_HANDLE(self) Get_RenderTexture_Struct(self)
#include "graphics/render_target.inc"
#undef RT_FN
#undef RT_METHOD
#undef RT_HANDLE

/* Document-class: SFML::RenderTexture
 * An off-screen render target backed by a Texture: anything drawable can be
 * drawn onto it, then read back via #texture (after #display).
 *
 * The methods below are shared with Window via the render_target.inc
 * fragment (see ext/graphics/render_target.inc) and documented here
 * directly since the fragment hides their function bodies from the
 * doc-comment scanner. Window's own docs restate the same list.
 *
 * @!method srgb?
 *   @return [Boolean]
 * @!method clear_stencil(value)
 *   @return [self]
 * @!method clear_color_and_stencil(color, stencil)
 *   @return [self]
 * @!method viewport(view = nil)
 *   @return [Rect] the current viewport in pixels; +view+ defaults to the target's current view
 * @!method scissor(view = nil)
 *   @return [Rect] the current scissor rectangle in pixels; +view+ defaults to the target's current
 * view
 * @!method map_pixel_to_coords(point, view = nil)
 *   Converts a pixel position to world coordinates, using the inverse of the view transform.
 *   @return [Vector2]
 * @!method map_coords_to_pixel(point, view = nil)
 *   Converts a world position to pixel coordinates.
 *   @return [Vector2]
 * @!method push_gl_states
 *   @return [self]
 * @!method pop_gl_states
 *   @return [self]
 * @!method reset_gl_states
 *   @return [self]
 * @!method draw_primitives(vertices, primitive, state = nil)
 *   @return [self]
 * @!method draw_vertex_buffer_range(buffer, first, count, state = nil)
 *   @return [self]
 */
void Init_RenderTexture(VALUE rb_mSFML) {
    rb_cRenderTexture = rb_define_class_under(rb_mSFML, "RenderTexture", rb_cObject);

    rb_define_singleton_method(rb_cRenderTexture, "new", RenderTexture_new, -1);
    rb_define_singleton_method(rb_cRenderTexture, "maximum_antialiasing_level",
                               RenderTexture_maximum_antialiasing_level, 0);

    rb_define_method(rb_cRenderTexture, "size", RenderTexture_get_size, 0);
    rb_define_method(rb_cRenderTexture, "display", RenderTexture_display, 0);
    rb_define_method(rb_cRenderTexture, "clear", RenderTexture_clear, -1);
    rb_define_method(rb_cRenderTexture, "view", RenderTexture_get_view, 0);
    rb_define_method(rb_cRenderTexture, "default_view", RenderTexture_get_default_view, 0);
    rb_define_method(rb_cRenderTexture, "texture", RenderTexture_get_texture, 0);
    rb_define_method(rb_cRenderTexture, "smooth?", RenderTexture_is_smooth, 0);
    rb_define_method(rb_cRenderTexture, "repeated?", RenderTexture_is_repeated, 0);
    rb_define_method(rb_cRenderTexture, "generate_mipmap", RenderTexture_generate_mipmap, 0);

    rb_define_method(rb_cRenderTexture, "view=", RenderTexture_set_view, 1);
    rb_define_method(rb_cRenderTexture, "active=", RenderTexture_set_active, 1);
    rb_define_method(rb_cRenderTexture, "smooth=", RenderTexture_set_smooth, 1);
    rb_define_method(rb_cRenderTexture, "repeated=", RenderTexture_set_repeated, 1);

    rb_define_method(rb_cRenderTexture, "draw", RenderTexture_draw, -1);

    RenderTexture_define_render_target_methods(rb_cRenderTexture);
}

VALUE Get_Klass_RenderTexture(void) {
    return rb_cRenderTexture;
}

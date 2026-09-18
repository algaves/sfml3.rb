#include "graphics/target.h"

#include <stdio.h>
#include <stdlib.h>

#include "graphics/render_state.h"
#include "graphics/render_texture.h"
#include "graphics/view.h"
#include "window/window.h"
#include "core/exceptions.h"
#include "core/macros.h"

static VALUE rb_cTarget;

static Target* Target_create(TargetType type, void* handle) {
    Target* target = malloc(sizeof(Target));

    target->type = type;
    target->handle = handle;

    return target;
}

static void RenderTarget_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t RenderTarget_data_type = {
    .wrap_struct_name = "SFML::Target",
    .function = {.dmark = NULL, .dfree = RenderTarget_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

/* call-seq:
 *   Target.new(window) -> Target
 *
 * Wraps +window+ (a Window) as a generic render target.
 *
 * @return [Target]
 * @raise [TypeError] if +window+ is not a Window
 */
static VALUE RenderTarget_new(VALUE klass, VALUE rb_window) {
    VALUE self;
    Target* target;

    if (!rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        raise_invalid_argument_class(Get_Klass_Window());
    }

    target = Target_create(SFML_TARGET_WINDOW, Get_Window_Struct(rb_window));
    self = TypedData_Wrap_Struct(klass, &RenderTarget_data_type, target);

    rb_obj_call_init(self, 0, NULL);

    return self;
}

static VALUE RenderTarget_init(VALUE self) {
    return self;
}

/* call-seq:
 *   draw(drawable, state) -> void
 *
 * Draws +drawable+ (any object responding to +#draw+, i.e. including
 * Drawable) onto this target using +state+ (a RenderState).
 *
 * @return [nil]
 * @raise [TypeError] if +state+ is not a RenderState
 */
static VALUE RenderTarget_draw(VALUE self, VALUE rb_drawable, VALUE rb_state) {
    if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
        raise_invalid_argument_class(Get_Klass_RenderState());
    }

    return rb_funcall(rb_drawable, rb_intern("draw"), 2, self, rb_state);
}

/* call-seq:
 *   view=(value) -> nil
 *
 * @return [nil]
 * @raise [TypeError] if +value+ is not a View
 */
static VALUE RenderTarget_set_view(VALUE self, VALUE rb_view) {
    Target* target = Get_Target_Struct(self);

    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        raise_invalid_argument_class(Get_Klass_View());
    }

    if (target->type == SFML_TARGET_WINDOW) {
        sfRenderWindow_setView((sfRenderWindow*)target->handle, Get_View_Struct(rb_view));
    } else {
        sfRenderTexture_setView((sfRenderTexture*)target->handle, Get_View_Struct(rb_view));
    }

    return Qnil;
}

/* call-seq: view -> View
 *
 * @return [View] a copy of the target's current view
 */
static VALUE RenderTarget_get_view(VALUE self) {
    Target* target = Get_Target_Struct(self);
    sfView* view;

    if (target->type == SFML_TARGET_WINDOW) {
        view = sfView_copy(sfRenderWindow_getView((sfRenderWindow*)target->handle));
    } else {
        view = sfView_copy(sfRenderTexture_getView((sfRenderTexture*)target->handle));
    }

    return Get_Casting_View(view);
}

/* Document-class: SFML::Target
 * A generic handle onto whatever can be drawn to -- a Window or a
 * RenderTexture -- used by Drawable#draw so drawable objects don't need to
 * know which concrete kind of target they're being drawn onto.
 *
 * @!attribute view
 *   @return [View]
 */
void Init_Target(VALUE rb_mSFML) {
    rb_cTarget = rb_define_class_under(rb_mSFML, "Target", rb_cObject);

    rb_define_singleton_method(rb_cTarget, "new", RenderTarget_new, 1);

    // methods
    rb_define_method(rb_cTarget, "initialize", RenderTarget_init, 0);
    rb_define_method(rb_cTarget, "draw", RenderTarget_draw, 2);

    // setters
    rb_define_method(rb_cTarget, "view=", RenderTarget_set_view, 1);

    // getters
    rb_define_method(rb_cTarget, "view", RenderTarget_get_view, 0);
}

VALUE Get_Klass_Target(void) {
    return rb_cTarget;
}

Target* Get_Target_Struct(VALUE self) {
    Target* ptr;
    TypedData_Get_Struct(self, Target, &RenderTarget_data_type, ptr);
    return ptr;
}

VALUE Get_New_Target(VALUE rb_window) {
    return RenderTarget_new(Get_Klass_Target(), rb_window);
}

VALUE Get_New_Target_From_RenderTexture(VALUE rb_render_texture) {
    VALUE self;
    Target* target;

    if (!rb_obj_is_kind_of(rb_render_texture, Get_Klass_RenderTexture())) {
        raise_invalid_argument_class(Get_Klass_RenderTexture());
    }

    target = Target_create(SFML_TARGET_TEXTURE, Get_RenderTexture_Struct(rb_render_texture));
    self = TypedData_Wrap_Struct(rb_cTarget, &RenderTarget_data_type, target);

    return self;
}

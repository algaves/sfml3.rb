#include "graphics/target.h"

#include <stdio.h>
#include <stdlib.h>
#include "graphics/circle.h"
#include "graphics/render_state.h"
#include "core/exceptions.h"

#include "graphics/transform.h"
#include "graphics/transformable.h"
#include "graphics/view.h"
#include "window/window.h"
#include "system/vec2.h"
#include "graphics/rect.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cTarget;

static Target *RenderTarget_create(sfRenderWindow *c_window) {
    Target *target = malloc(sizeof(Target));

    target->window = c_window;

    return target;
}

static void RenderTarget_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t RenderTarget_data_type = {
    .wrap_struct_name = "SFML::Target",
    .function = {.dmark = NULL, .dfree = RenderTarget_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE RenderTarget_new(VALUE klass, VALUE rb_window) {
    VALUE self;
    Target *target;

    if (!rb_obj_is_kind_of(rb_window, Get_Klass_Window())) {
        rb_raise(rb_eArgError, "Expected a Window object");
    }

    target = RenderTarget_create(Get_Window_Struct(rb_window));
    self = TypedData_Wrap_Struct(klass, &RenderTarget_data_type, target);

    VALUE argv[1] = {rb_window};
    rb_obj_call_init(self, 1, argv);

    return self;
}

static VALUE RenderTarget_init(VALUE self, VALUE rb_window) {
    return self;
}

static VALUE RenderTarget_draw(VALUE self, VALUE rb_drawable, VALUE rb_state) {
    Target *target = Get_Target_Struct(self);

    if (rb_obj_is_kind_of(rb_drawable, Get_Klass_Circle())) {
        // The drawable is checked just above; the state was not, which left the
        // only unguarded unwrap of a caller-supplied object in the extension.
        if (!rb_obj_is_kind_of(rb_state, Get_Klass_RenderState())) {
            raise_invalid_argument_class(Get_Klass_RenderState());
        }

        sfRenderWindow_drawCircleShape(target->window, Get_Circle_Struct(rb_drawable), Get_RenderState_Struct(rb_state));
    } else {
        rb_funcall(rb_drawable, rb_intern("draw"), 2, self, rb_state);
    }

    return self;
}

static VALUE RenderTarget_set_view(VALUE self, VALUE rb_view) {
    if (!rb_obj_is_kind_of(rb_view, Get_Klass_View())) {
        rb_raise(rb_eArgError, "Expected a View object");
    }

    sfRenderWindow_setView(((Target *) Get_Target_Struct(self))->window, Get_View_Struct(rb_view));

    return Qnil;
}

static VALUE RenderTarget_get_view(VALUE self) {
    return Get_Casting_View(sfView_copy(sfRenderWindow_getView(((Target *) Get_Target_Struct(self))->window)));
}

void Init_Target(VALUE rb_module) {
    rb_cTarget = rb_define_class_under(rb_module, "Target", rb_cObject);

    rb_define_singleton_method(rb_cTarget, "new", RenderTarget_new, 1);

    // methods
    rb_define_method(rb_cTarget, "initialize", RenderTarget_init, 1);
    rb_define_method(rb_cTarget, "draw", RenderTarget_draw, 2);

    // setters
    rb_define_method(rb_cTarget, "view=", RenderTarget_set_view, 1);

    // getters
    rb_define_method(rb_cTarget, "view", RenderTarget_get_view, 0);
}

VALUE Get_Klass_Target(void) {
    return rb_cTarget;
}

void *Get_Target_Struct(VALUE self) {
    Target *ptr;
    TypedData_Get_Struct(self, Target, &RenderTarget_data_type, ptr);
    return ptr;
}

VALUE Get_New_Target(VALUE rb_window) {
    return RenderTarget_new(Get_Klass_Target(), rb_window);
}
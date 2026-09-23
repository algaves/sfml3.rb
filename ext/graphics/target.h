#ifndef SFML_RB_GRAPHICS_TARGET_H
#define SFML_RB_GRAPHICS_TARGET_H

#include <ruby.h>

#include "core/sfml.h"

typedef enum { SFML_TARGET_WINDOW, SFML_TARGET_TEXTURE } TargetType;

/* A borrowed view onto whatever a draw call is headed for: the kind plus the
   opaque handle. Built on the stack by Get_RenderTarget_View, never owned. */
typedef struct {
    TargetType type;
    void* handle;
} TargetView;

/* The legacy generic wrapper behind SF::Graphics::Target. It owns only the borrow, so
   it keeps its source alive for as long as it references it. */
typedef struct {
    TargetType type;
    void* handle;
    /* The Window/RenderWindow/RenderTexture `handle` is borrowed from, kept
       alive for as long as this Target references it -- see Target_mark. */
    VALUE rb_source;
} Target;

/* Dispatch a draw call to whichever underlying render surface this view
   wraps. Pass NULL states to use the defaults; never hand CSFML a NULL. */
#define TARGET_DRAW(view, window_fn, texture_fn, object, states)                                   \
    do {                                                                                           \
        const sfRenderStates* sfml_states = (states) ? (states) : &sfRenderStates_default;         \
        if ((view).type == SFML_TARGET_WINDOW) {                                                   \
            window_fn((sfRenderWindow*)(view).handle, (object), sfml_states);                      \
        } else {                                                                                   \
            texture_fn((sfRenderTexture*)(view).handle, (object), sfml_states);                    \
        }                                                                                          \
    } while (0)

void Init_Target(VALUE rb_module);

VALUE Get_Klass_Target(void);

Target* Get_Target_Struct(VALUE self);

VALUE Get_New_Target(VALUE rb_window);

/* The SF::Graphics::RenderTarget module, included by RenderWindow and RenderTexture. */
VALUE Get_Module_RenderTarget(void);

/* Resolves any accepted draw target -- a legacy Target, a RenderWindow or a
   RenderTexture -- to its kind and handle, raising TypeError otherwise. */
TargetView Get_RenderTarget_View(VALUE self);

#endif // SFML_RB_GRAPHICS_TARGET_H

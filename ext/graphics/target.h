#ifndef SFML_RB_GRAPHICS_TARGET_H
#define SFML_RB_GRAPHICS_TARGET_H

#include <ruby.h>

#include "core/sfml.h"

typedef enum {
    SFML_TARGET_WINDOW,
    SFML_TARGET_TEXTURE
} TargetType;

typedef struct {
    TargetType type;
    void *handle;
} Target;

/* Dispatch a draw call to whichever underlying render surface this target
   wraps. Pass NULL states to use the defaults; never hand CSFML a NULL. */
#define TARGET_DRAW(target, window_fn, texture_fn, object, states)                        \
    do {                                                                                  \
        const sfRenderStates *sfml_states = (states) ? (states) : &sfRenderStates_default; \
        if ((target)->type == SFML_TARGET_WINDOW) {                                       \
            window_fn((sfRenderWindow *) (target)->handle, (object), sfml_states);        \
        } else {                                                                          \
            texture_fn((sfRenderTexture *) (target)->handle, (object), sfml_states);      \
        }                                                                                 \
    } while (0)

void Init_Target(VALUE rb_module);

VALUE Get_Klass_Target(void);

Target *Get_Target_Struct(VALUE self);

VALUE Get_New_Target(VALUE rb_window);

VALUE Get_New_Target_From_RenderTexture(VALUE rb_render_texture);

#endif //SFML_RB_GRAPHICS_TARGET_H

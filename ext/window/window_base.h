#ifndef SFML_RB_WINDOW_WINDOW_BASE_H
#define SFML_RB_WINDOW_WINDOW_BASE_H

#include <ruby.h>

#include "core/sfml.h"

/* Which CSFML handle a wrapper owns. SFML::Window and SFML::RenderWindow wrap
   an sfRenderWindow and share every method with sfWindowBase_, but CSFML keys
   its destroy off the concrete type, so dfree has to know which one to call. */
typedef enum { SFML_WINDOW_KIND_BASE, SFML_WINDOW_KIND_WINDOW } WindowKind;

/* One C struct backs every window class. SFML::WindowBase owns an
   sfWindowBase* and SFML::Window/RenderWindow an sfRenderWindow*; the handle is
   stored as void* and read back through the typed accessors below, one per
   class, so no cast ever crosses the two. */
typedef struct {
    void* handle;
    WindowKind kind;
    /* sfWindowBase_setMouseCursor requires the Cursor to stay alive for as long
       as it's in use by the window (CSFML/SFML docs), so the wrapper retains it
       -- which is why Window_mark exists, unlike a bare pointer. */
    VALUE rb_cursor;
} Window;

void Init_WindowBase(VALUE rb_module);

VALUE Get_Klass_WindowBase(void);

Window* Get_Window_Data(VALUE self);

sfWindowBase* Get_WindowBase_Struct(VALUE self);

sfRenderWindow* Get_Window_Struct(VALUE self);

/* Allocates a wrapper with a NULL handle; registered as the allocator of every
   window class so the T_DATA default-allocator warning stays quiet. */
VALUE Window_alloc(VALUE klass);

/* Wraps a freshly created handle, tagging it with the destroy to use. */
VALUE Window_wrap_handle(VALUE klass, void* handle, WindowKind kind);

#endif // SFML_RB_WINDOW_WINDOW_BASE_H

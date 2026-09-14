#include <ruby.h>
#include <stdio.h>

#include "core/macros.h"
#include "graphics/transform.h"
#include "graphics/drawable.h"
#include "graphics/color.h"
#include "graphics/transformable.h"
#include "system/clock.h"
#include "graphics/target.h"
#include "graphics/render_state.h"
#include "window/event.h"
#include "window/window.h"
#include "graphics/view.h"
#include "window/video_mode.h"
#include "graphics/circle.h"


//  C Naming Convention:
//
//    Struct              TitleCase
//    Struct Members      lower_case or lowerCase
//
//    Enum                ETitleCase
//    Enum Members        ALL_CAPS or lowerCase
//
//    Public functions    pfx_TitleCase (pfx = two or three letter module prefix)
//    Private functions   TitleCase
//    Trivial variables   i,x,n,f etc...
//    Local variables     lower_case or lowerCase
//    Global variables    g_lowerCase or g_lower_case (searchable by g_ prefix)

static VALUE rb_mExt;

void Init_sfml_ext(void) {
    rb_mExt = rb_define_module("SFML");

    Init_Drawable(rb_mExt);
    Init_Transform(rb_mExt);
    Init_Transformable(rb_mExt);
    Init_Clock(rb_mExt);
    Init_Target(rb_mExt);
    Init_RenderState(rb_mExt);
    Init_Circle(rb_mExt);
    Init_Event(rb_mExt);
    Init_VideoMode(rb_mExt);
    Init_View(rb_mExt);
    Init_Window(rb_mExt);
}

#ifndef SFML_RB_GRAPHICS_VIEW_H
#define SFML_RB_GRAPHICS_VIEW_H

#include <ruby.h>

void Init_View(VALUE rb_module);

void *Get_View_Struct(VALUE self);

VALUE Get_Klass_View();

VALUE Get_Casting_View(void *ptr);

#endif //SFML_RB_GRAPHICS_VIEW_H

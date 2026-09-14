#ifndef SFML_RB_GRAPHICS_IMAGE_H
#define SFML_RB_GRAPHICS_IMAGE_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Image(VALUE rb_module);

VALUE Get_Klass_Image(void);

const sfImage *Get_Image_Struct(VALUE self);

VALUE image_from_c(sfImage *image);

#endif //SFML_RB_GRAPHICS_IMAGE_H

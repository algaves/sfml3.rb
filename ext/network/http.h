#ifndef SFML_RB_NETWORK_HTTP_H
#define SFML_RB_NETWORK_HTTP_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Http(VALUE rb_module);

VALUE Get_Klass_Http(void);
VALUE Get_Klass_HttpRequest(void);
VALUE Get_Klass_HttpResponse(void);

void *Get_Http_Struct(VALUE self);
void *Get_HttpRequest_Struct(VALUE self);
void *Get_HttpResponse_Struct(VALUE self);

#endif //SFML_RB_NETWORK_HTTP_H

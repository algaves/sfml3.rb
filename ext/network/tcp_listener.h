#ifndef SFML_RB_NETWORK_TCP_LISTENER_H
#define SFML_RB_NETWORK_TCP_LISTENER_H

#include <ruby.h>

#include "core/sfml.h"

void Init_TcpListener(VALUE rb_module);

VALUE Get_Klass_TcpListener(void);

void *Get_TcpListener_Struct(VALUE self);

#endif //SFML_RB_NETWORK_TCP_LISTENER_H

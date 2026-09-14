#ifndef SFML_RB_NETWORK_SOCKET_SELECTOR_H
#define SFML_RB_NETWORK_SOCKET_SELECTOR_H

#include <ruby.h>

#include "core/sfml.h"

void Init_SocketSelector(VALUE rb_module);

VALUE Get_Klass_SocketSelector(void);

void *Get_SocketSelector_Struct(VALUE self);

#endif //SFML_RB_NETWORK_SOCKET_SELECTOR_H

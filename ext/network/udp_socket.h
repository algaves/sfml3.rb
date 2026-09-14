#ifndef SFML_RB_NETWORK_UDP_SOCKET_H
#define SFML_RB_NETWORK_UDP_SOCKET_H

#include <ruby.h>

#include "core/sfml.h"

void Init_UdpSocket(VALUE rb_module);

VALUE Get_Klass_UdpSocket(void);

void *Get_UdpSocket_Struct(VALUE self);

#endif //SFML_RB_NETWORK_UDP_SOCKET_H

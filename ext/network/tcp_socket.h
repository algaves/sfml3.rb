#ifndef SFML_RB_NETWORK_TCP_SOCKET_H
#define SFML_RB_NETWORK_TCP_SOCKET_H

#include <ruby.h>

#include "core/sfml.h"

void Init_TcpSocket(VALUE rb_module);

VALUE Get_Klass_TcpSocket(void);

void *Get_TcpSocket_Struct(VALUE self);

/* Wraps a socket returned by sfTcpListener_accept, taking ownership. */
VALUE tcp_socket_from_handle(sfTcpSocket *socket);

#endif //SFML_RB_NETWORK_TCP_SOCKET_H

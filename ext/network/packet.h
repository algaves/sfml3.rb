#ifndef SFML_RB_NETWORK_PACKET_H
#define SFML_RB_NETWORK_PACKET_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Packet(VALUE rb_module);

VALUE Get_Klass_Packet(void);

void *Get_Packet_Struct(VALUE self);

#endif //SFML_RB_NETWORK_PACKET_H

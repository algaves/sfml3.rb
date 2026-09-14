#ifndef SFML_RB_NETWORK_IP_ADDRESS_H
#define SFML_RB_NETWORK_IP_ADDRESS_H

#include <ruby.h>

#include "core/sfml.h"

void Init_IpAddress(VALUE rb_module);

VALUE Get_Klass_IpAddress(void);

void *Get_IpAddress_Struct(VALUE self);

VALUE ip_address_to_rb(sfIpAddress address);

/* Accepts an SFML::IpAddress, a dotted-quad String, or nil (which yields
   `fallback`, so callers can implement optional address arguments). */
sfIpAddress ip_address_from_rb(VALUE rb_address, sfIpAddress fallback);

#endif //SFML_RB_NETWORK_IP_ADDRESS_H

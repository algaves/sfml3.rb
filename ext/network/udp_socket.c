#include "network/udp_socket.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/macros.h"
#include "network/ip_address.h"
#include "network/network_enums.h"
#include "network/packet.h"

typedef struct {
    sfUdpSocket *handle;
} UdpSocket;

static VALUE rb_cUdpSocket;

static void UdpSocket_free(void *ptr) {
    UdpSocket *socket = ptr;

    if (socket->handle != NULL) {
        sfUdpSocket_destroy(socket->handle);
    }

    free(socket);
}

static const rb_data_type_t UdpSocket_data_type = {
    .wrap_struct_name = "SFML::UdpSocket",
    .function = {.dmark = NULL, .dfree = UdpSocket_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE UdpSocket_wrap(VALUE klass, sfUdpSocket *handle) {
    UdpSocket *ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create UDP socket");
    }

    ptr = malloc(sizeof(UdpSocket));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &UdpSocket_data_type, ptr);
}

static VALUE UdpSocket_new(VALUE klass) {
    return UdpSocket_wrap(klass, sfUdpSocket_create());
}

static VALUE UdpSocket_any_port(VALUE klass) {
    return UINT2NUM(sfUdpSocket_anyPort());
}

static VALUE UdpSocket_max_datagram_size(VALUE klass) {
    return UINT2NUM(sfUdpSocket_maxDatagramSize());
}

static VALUE UdpSocket_blocking(VALUE self) {
    return BOOL2RB(sfUdpSocket_isBlocking(Get_UdpSocket_Struct(self)));
}

static VALUE UdpSocket_set_blocking(VALUE self, VALUE rb_value) {
    sfUdpSocket_setBlocking(Get_UdpSocket_Struct(self), RTEST(rb_value));
    return rb_value;
}

static VALUE UdpSocket_local_port(VALUE self) {
    return UINT2NUM(sfUdpSocket_getLocalPort(Get_UdpSocket_Struct(self)));
}

static VALUE UdpSocket_bind(int argc, VALUE *argv, VALUE self) {
    VALUE rb_port, rb_address;

    rb_scan_args(argc, argv, "11", &rb_port, &rb_address);

    return ID2SYM(rb_intern(socket_status_name(sfUdpSocket_bind(
        Get_UdpSocket_Struct(self), (unsigned short) NUM2INT(rb_port),
        ip_address_from_rb(rb_address, sfIpAddress_Any)))));
}

static VALUE UdpSocket_unbind(VALUE self) {
    sfUdpSocket_unbind(Get_UdpSocket_Struct(self));
    return self;
}

static VALUE UdpSocket_send(VALUE self, VALUE rb_data, VALUE rb_address, VALUE rb_port) {
    StringValue(rb_data);

    return ID2SYM(rb_intern(socket_status_name(sfUdpSocket_send(
        Get_UdpSocket_Struct(self), RSTRING_PTR(rb_data), (size_t) RSTRING_LEN(rb_data),
        ip_address_from_rb(rb_address, sfIpAddress_None), (unsigned short) NUM2INT(rb_port)))));
}

/* Returns [data, IpAddress, port, status]; the address and port are those of
   the sender. */
static VALUE UdpSocket_receive(int argc, VALUE *argv, VALUE self) {
    VALUE rb_max_length;
    long max_length = 1024;
    char *buffer;
    size_t received = 0;
    sfIpAddress remote_address = sfIpAddress_None;
    unsigned short remote_port = 0;
    sfSocketStatus status;
    VALUE rb_result;

    rb_scan_args(argc, argv, "01", &rb_max_length);

    if (!NIL_P(rb_max_length)) {
        max_length = NUM2LONG(rb_max_length);
    }

    if (max_length <= 0) {
        rb_raise(rb_eArgError, "maximum length must be positive");
    }

    buffer = malloc((size_t) max_length);

    status = sfUdpSocket_receive(Get_UdpSocket_Struct(self), buffer, (size_t) max_length, &received,
                                 &remote_address, &remote_port);

    rb_result = rb_ary_new_capa(4);
    rb_ary_push(rb_result, rb_str_new(buffer, (long) received));
    rb_ary_push(rb_result, ip_address_to_rb(remote_address));
    rb_ary_push(rb_result, UINT2NUM(remote_port));
    rb_ary_push(rb_result, ID2SYM(rb_intern(socket_status_name(status))));

    free(buffer);

    return rb_result;
}

static VALUE UdpSocket_send_packet(VALUE self, VALUE rb_packet, VALUE rb_address, VALUE rb_port) {
    if (!rb_obj_is_kind_of(rb_packet, Get_Klass_Packet())) {
        rb_raise(rb_eTypeError, "expected an SFML::Packet");
    }

    return ID2SYM(rb_intern(socket_status_name(sfUdpSocket_sendPacket(
        Get_UdpSocket_Struct(self), Get_Packet_Struct(rb_packet),
        ip_address_from_rb(rb_address, sfIpAddress_None), (unsigned short) NUM2INT(rb_port)))));
}

/* Returns [IpAddress, port, status]. */
static VALUE UdpSocket_receive_packet(VALUE self, VALUE rb_packet) {
    sfIpAddress remote_address = sfIpAddress_None;
    unsigned short remote_port = 0;
    sfSocketStatus status;
    VALUE rb_result;

    if (!rb_obj_is_kind_of(rb_packet, Get_Klass_Packet())) {
        rb_raise(rb_eTypeError, "expected an SFML::Packet");
    }

    status = sfUdpSocket_receivePacket(Get_UdpSocket_Struct(self), Get_Packet_Struct(rb_packet),
                                       &remote_address, &remote_port);

    rb_result = rb_ary_new_capa(3);
    rb_ary_push(rb_result, ip_address_to_rb(remote_address));
    rb_ary_push(rb_result, UINT2NUM(remote_port));
    rb_ary_push(rb_result, ID2SYM(rb_intern(socket_status_name(status))));

    return rb_result;
}

void Init_UdpSocket(VALUE rb_module) {
    rb_cUdpSocket = rb_define_class_under(rb_module, "UdpSocket", rb_cObject);

    rb_define_singleton_method(rb_cUdpSocket, "new", UdpSocket_new, 0);
    rb_define_singleton_method(rb_cUdpSocket, "any_port", UdpSocket_any_port, 0);
    rb_define_singleton_method(rb_cUdpSocket, "max_datagram_size", UdpSocket_max_datagram_size, 0);

    rb_define_method(rb_cUdpSocket, "blocking?", UdpSocket_blocking, 0);
    rb_define_method(rb_cUdpSocket, "blocking=", UdpSocket_set_blocking, 1);
    rb_define_method(rb_cUdpSocket, "local_port", UdpSocket_local_port, 0);
    rb_define_method(rb_cUdpSocket, "bind", UdpSocket_bind, -1);
    rb_define_method(rb_cUdpSocket, "unbind", UdpSocket_unbind, 0);
    rb_define_method(rb_cUdpSocket, "send", UdpSocket_send, 3);
    rb_define_method(rb_cUdpSocket, "receive", UdpSocket_receive, -1);
    rb_define_method(rb_cUdpSocket, "send_packet", UdpSocket_send_packet, 3);
    rb_define_method(rb_cUdpSocket, "receive_packet", UdpSocket_receive_packet, 1);
}

VALUE Get_Klass_UdpSocket(void) {
    return rb_cUdpSocket;
}

void *Get_UdpSocket_Struct(VALUE self) {
    UdpSocket *ptr;
    TypedData_Get_Struct(self, UdpSocket, &UdpSocket_data_type, ptr);
    return ptr->handle;
}

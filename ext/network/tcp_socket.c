#include "network/tcp_socket.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/macros.h"
#include "network/ip_address.h"
#include "network/network_enums.h"
#include "network/packet.h"
#include "system/time.h"

typedef struct {
    sfTcpSocket* handle;
} TcpSocket;

static VALUE rb_cTcpSocket;

static void TcpSocket_free(void* ptr) {
    TcpSocket* socket = ptr;

    if (socket->handle != NULL) {
        sfTcpSocket_destroy(socket->handle);
    }

    free(socket);
}

static const rb_data_type_t TcpSocket_data_type = {
    .wrap_struct_name = "SF::Network::TcpSocket",
    .function = {.dmark = NULL, .dfree = TcpSocket_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE TcpSocket_wrap(VALUE klass, sfTcpSocket* handle) {
    TcpSocket* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create TCP socket");
    }

    ptr = malloc(sizeof(TcpSocket));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &TcpSocket_data_type, ptr);
}

VALUE tcp_socket_from_handle(sfTcpSocket* socket) {
    return TcpSocket_wrap(rb_cTcpSocket, socket);
}

static VALUE TcpSocket_alloc(VALUE klass) {
    return TcpSocket_wrap(klass, sfTcpSocket_create());
}

/* call-seq:
 *   TcpSocket.new -> TcpSocket
 *
 * Creates a new, unconnected TCP socket.
 *
 * @return [TcpSocket] a new, unconnected socket
 */
static VALUE TcpSocket_initialize(VALUE self) {
    return self;
}

/* call-seq: blocking? -> true or false
 *
 * Returns +true+ if the socket is in blocking mode.
 *
 * @return [Boolean]
 */
static VALUE TcpSocket_blocking(VALUE self) {
    return BOOL2RB(sfTcpSocket_isBlocking(Get_TcpSocket_Struct(self)));
}

/* call-seq:
 *   blocking=(value) -> value
 *
 * Enables or disables blocking mode according to +value+.
 *
 * @return [Boolean] +value+
 */
static VALUE TcpSocket_set_blocking(VALUE self, VALUE rb_value) {
    sfTcpSocket_setBlocking(Get_TcpSocket_Struct(self), RTEST(rb_value));
    return rb_value;
}

/* call-seq: local_port -> Integer
 *
 * Returns the local port the socket is bound to.
 *
 * @return [Integer] the local port the socket is bound to, or 0 if it
 *   isn't connected
 */
static VALUE TcpSocket_local_port(VALUE self) {
    return UINT2NUM(sfTcpSocket_getLocalPort(Get_TcpSocket_Struct(self)));
}

/* call-seq: remote_address -> IpAddress
 *
 * Returns the address of the remote peer.
 *
 * @return [IpAddress] the address of the remote peer, or +IpAddress::NONE+
 *   if not connected
 */
static VALUE TcpSocket_remote_address(VALUE self) {
    return ip_address_to_rb(sfTcpSocket_getRemoteAddress(Get_TcpSocket_Struct(self)));
}

/* call-seq: remote_port -> Integer
 *
 * Returns the port of the remote peer.
 *
 * @return [Integer] the port of the remote peer, or 0 if not connected
 */
static VALUE TcpSocket_remote_port(VALUE self) {
    return UINT2NUM(sfTcpSocket_getRemotePort(Get_TcpSocket_Struct(self)));
}

/* call-seq:
 *   connect(address, port, timeout = Time.zero) -> Symbol
 *
 * Connects to a remote peer, blocking (unless #blocking? is +false+) until
 * connected or +timeout+ elapses.
 *
 * @return [Symbol] a SocketStatus name, +:done+ on success
 */
static VALUE TcpSocket_connect(int argc, VALUE* argv, VALUE self) {
    VALUE rb_address, rb_port, rb_timeout;
    sfTime timeout = sfTime_Zero;

    rb_scan_args(argc, argv, "21", &rb_address, &rb_port, &rb_timeout);

    if (!NIL_P(rb_timeout)) {
        timeout = time_from_rb(rb_timeout);
    }

    return ID2SYM(rb_intern(socket_status_name(sfTcpSocket_connect(
        Get_TcpSocket_Struct(self), ip_address_from_rb(rb_address, sfIpAddress_None),
        (unsigned short)NUM2INT(rb_port), timeout))));
}

/* call-seq: disconnect -> self
 *
 * Closes the connection.
 *
 * @return [self]
 */
static VALUE TcpSocket_disconnect(VALUE self) {
    sfTcpSocket_disconnect(Get_TcpSocket_Struct(self));
    return self;
}

/* call-seq:
 *   send(data) -> Symbol
 *
 * Sends the full contents of +data+, blocking (unless #blocking? is
 * +false+) until it is all sent. Prefer #send_partial for non-blocking
 * sockets, since this can return +:not_ready+ having sent only part of
 * the data with no way to know how much.
 *
 * @return [Symbol] a SocketStatus name, +:done+ on success
 */
static VALUE TcpSocket_send(VALUE self, VALUE rb_data) {
    StringValue(rb_data);

    return ID2SYM(rb_intern(socket_status_name(sfTcpSocket_send(
        Get_TcpSocket_Struct(self), RSTRING_PTR(rb_data), (size_t)RSTRING_LEN(rb_data)))));
}

/* call-seq:
 *   send_partial(data) -> [Integer, Symbol]
 *
 * Like #send, but may send only part of +data+ and reports how much.
 *
 * @return [Array(Integer, Symbol)] the number of bytes actually sent, and
 *   a SocketStatus name
 */
static VALUE TcpSocket_send_partial(VALUE self, VALUE rb_data) {
    size_t sent = 0;
    sfSocketStatus status;
    VALUE rb_result;

    StringValue(rb_data);

    status = sfTcpSocket_sendPartial(Get_TcpSocket_Struct(self), RSTRING_PTR(rb_data),
                                     (size_t)RSTRING_LEN(rb_data), &sent);

    rb_result = rb_ary_new_capa(2);
    rb_ary_push(rb_result, SIZET2NUM(sent));
    rb_ary_push(rb_result, ID2SYM(rb_intern(socket_status_name(status))));

    return rb_result;
}

/* call-seq:
 *   receive(max_length = 1024) -> [String, Symbol]
 *
 * Receive needs a maximum size up front because CSFML fills a caller buffer.
 * Returns [data, status], where data is empty unless the status is :done.
 *
 * @return [Array(String, Symbol)] the bytes received, and a SocketStatus name
 * @raise [ArgumentError] if +max_length+ isn't positive
 */
static VALUE TcpSocket_receive(int argc, VALUE* argv, VALUE self) {
    VALUE rb_max_length;
    long max_length = 1024;
    char* buffer;
    size_t received = 0;
    sfSocketStatus status;
    VALUE rb_result;

    rb_scan_args(argc, argv, "01", &rb_max_length);

    if (!NIL_P(rb_max_length)) {
        max_length = NUM2LONG(rb_max_length);
    }

    if (max_length <= 0) {
        rb_raise(rb_eArgError, "maximum length must be positive");
    }

    buffer = malloc((size_t)max_length);

    if (buffer == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate receive buffer");
    }

    status = sfTcpSocket_receive(Get_TcpSocket_Struct(self), buffer, (size_t)max_length, &received);

    rb_result = rb_ary_new_capa(2);
    rb_ary_push(rb_result, rb_str_new(buffer, (long)received));
    rb_ary_push(rb_result, ID2SYM(rb_intern(socket_status_name(status))));

    free(buffer);

    return rb_result;
}

/* call-seq:
 *   send_packet(packet) -> Symbol
 *
 * Sends +packet+ over the connection.
 *
 * @return [Symbol] a SocketStatus name, +:done+ on success
 * @raise [TypeError] if +packet+ is not an SF::Network::Packet
 */
static VALUE TcpSocket_send_packet(VALUE self, VALUE rb_packet) {
    if (!rb_obj_is_kind_of(rb_packet, Get_Klass_Packet())) {
        rb_raise(rb_eTypeError, "expected an SF::Network::Packet");
    }

    return ID2SYM(rb_intern(socket_status_name(
        sfTcpSocket_sendPacket(Get_TcpSocket_Struct(self), Get_Packet_Struct(rb_packet)))));
}

/* call-seq:
 *   receive_packet(packet) -> Symbol
 *
 * Fills +packet+ (replacing its previous contents) with the next packet
 * received.
 *
 * @return [Symbol] a SocketStatus name, +:done+ on success
 * @raise [TypeError] if +packet+ is not an SF::Network::Packet
 */
static VALUE TcpSocket_receive_packet(VALUE self, VALUE rb_packet) {
    if (!rb_obj_is_kind_of(rb_packet, Get_Klass_Packet())) {
        rb_raise(rb_eTypeError, "expected an SF::Network::Packet");
    }

    return ID2SYM(rb_intern(socket_status_name(
        sfTcpSocket_receivePacket(Get_TcpSocket_Struct(self), Get_Packet_Struct(rb_packet)))));
}

/* Document-class: SF::Network::TcpSocket
 * A connection-oriented, reliable, byte-stream socket for TCP communication.
 * Connect with #connect (or receive one already connected from
 * TcpListener#accept), then send/receive raw bytes or Packet instances.
 */
void Init_TcpSocket(VALUE rb_mNetwork) {
    rb_cTcpSocket = rb_define_class_under(rb_mNetwork, "TcpSocket", rb_cObject);
    rb_define_alloc_func(rb_cTcpSocket, TcpSocket_alloc);

    rb_define_method(rb_cTcpSocket, "initialize", TcpSocket_initialize, 0);

    rb_define_method(rb_cTcpSocket, "blocking?", TcpSocket_blocking, 0);
    rb_define_method(rb_cTcpSocket, "blocking=", TcpSocket_set_blocking, 1);
    rb_define_method(rb_cTcpSocket, "local_port", TcpSocket_local_port, 0);
    rb_define_method(rb_cTcpSocket, "remote_address", TcpSocket_remote_address, 0);
    rb_define_method(rb_cTcpSocket, "remote_port", TcpSocket_remote_port, 0);
    rb_define_method(rb_cTcpSocket, "connect", TcpSocket_connect, -1);
    rb_define_method(rb_cTcpSocket, "disconnect", TcpSocket_disconnect, 0);
    rb_define_method(rb_cTcpSocket, "send", TcpSocket_send, 1);
    rb_define_method(rb_cTcpSocket, "send_partial", TcpSocket_send_partial, 1);
    rb_define_method(rb_cTcpSocket, "receive", TcpSocket_receive, -1);
    rb_define_method(rb_cTcpSocket, "send_packet", TcpSocket_send_packet, 1);
    rb_define_method(rb_cTcpSocket, "receive_packet", TcpSocket_receive_packet, 1);
}

VALUE Get_Klass_TcpSocket(void) {
    return rb_cTcpSocket;
}

void* Get_TcpSocket_Struct(VALUE self) {
    TcpSocket* ptr;
    TypedData_Get_Struct(self, TcpSocket, &TcpSocket_data_type, ptr);
    return ptr->handle;
}

#include "network/socket_selector.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"
#include "network/tcp_listener.h"
#include "network/tcp_socket.h"
#include "network/udp_socket.h"
#include "system/time.h"

typedef struct {
    sfSocketSelector* handle;
} SocketSelector;

static VALUE rb_cSocketSelector;

static void SocketSelector_free(void* ptr) {
    SocketSelector* selector = ptr;

    sfSocketSelector_destroy(selector->handle);
    free(selector);
}

static const rb_data_type_t SocketSelector_data_type = {
    .wrap_struct_name = "SF::Network::SocketSelector",
    .function = {.dmark = NULL, .dfree = SocketSelector_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE SocketSelector_wrap(VALUE klass, sfSocketSelector* handle) {
    SocketSelector* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create socket selector");
    }

    ptr = malloc(sizeof(SocketSelector));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &SocketSelector_data_type, ptr);
}

static VALUE SocketSelector_alloc(VALUE klass) {
    return SocketSelector_wrap(klass, sfSocketSelector_create());
}

/* call-seq:
 *   SocketSelector.new -> SocketSelector
 *
 * Creates a new, empty socket selector.
 *
 * @return [SocketSelector]
 */
static VALUE SocketSelector_initialize(VALUE self) {
    return self;
}

/* call-seq: copy -> SocketSelector
 *
 * Returns an independent copy of the selector.
 *
 * @return [SocketSelector] an independent copy with the same watched sockets
 */
static VALUE SocketSelector_copy(VALUE self) {
    return SocketSelector_wrap(Get_Klass_SocketSelector(),
                               sfSocketSelector_copy(Get_SocketSelector_Struct(self)));
}

/* call-seq:
 *   add(socket) -> self
 *
 * Adds a TcpListener, TcpSocket or UdpSocket to watch for activity.
 *
 * @return [self]
 * @raise [TypeError] if +socket+ is none of the above
 */
static VALUE SocketSelector_add(VALUE self, VALUE rb_socket) {
    sfSocketSelector* selector = Get_SocketSelector_Struct(self);

    if (rb_obj_is_kind_of(rb_socket, Get_Klass_TcpListener())) {
        sfSocketSelector_addTcpListener(selector, Get_TcpListener_Struct(rb_socket));
    } else if (rb_obj_is_kind_of(rb_socket, Get_Klass_TcpSocket())) {
        sfSocketSelector_addTcpSocket(selector, Get_TcpSocket_Struct(rb_socket));
    } else if (rb_obj_is_kind_of(rb_socket, Get_Klass_UdpSocket())) {
        sfSocketSelector_addUdpSocket(selector, Get_UdpSocket_Struct(rb_socket));
    } else {
        rb_raise(rb_eTypeError, "expected an SF::Network::TcpListener, TcpSocket or UdpSocket");
    }

    return self;
}

/* call-seq:
 *   remove(socket) -> self
 *
 * Removes a socket previously added with #add.
 *
 * @return [self]
 * @raise [TypeError] if +socket+ is not a TcpListener, TcpSocket or UdpSocket
 */
static VALUE SocketSelector_remove(VALUE self, VALUE rb_socket) {
    sfSocketSelector* selector = Get_SocketSelector_Struct(self);

    if (rb_obj_is_kind_of(rb_socket, Get_Klass_TcpListener())) {
        sfSocketSelector_removeTcpListener(selector, Get_TcpListener_Struct(rb_socket));
    } else if (rb_obj_is_kind_of(rb_socket, Get_Klass_TcpSocket())) {
        sfSocketSelector_removeTcpSocket(selector, Get_TcpSocket_Struct(rb_socket));
    } else if (rb_obj_is_kind_of(rb_socket, Get_Klass_UdpSocket())) {
        sfSocketSelector_removeUdpSocket(selector, Get_UdpSocket_Struct(rb_socket));
    } else {
        rb_raise(rb_eTypeError, "expected an SF::Network::TcpListener, TcpSocket or UdpSocket");
    }

    return self;
}

/* call-seq: clear -> self
 *
 * Removes all watched sockets.
 *
 * @return [self]
 */
static VALUE SocketSelector_clear(VALUE self) {
    sfSocketSelector_clear(Get_SocketSelector_Struct(self));
    return self;
}

/* call-seq:
 *   wait(timeout = Time.zero) -> true or false
 *
 * Blocks until one of the watched sockets is ready to read, or +timeout+
 * elapses. A zero +timeout+ means wait forever.
 *
 * @return [Boolean] whether a socket became ready (+false+ on timeout)
 */
static VALUE SocketSelector_wait(int argc, VALUE* argv, VALUE self) {
    VALUE rb_timeout;
    sfTime timeout = sfTime_Zero;

    rb_scan_args(argc, argv, "01", &rb_timeout);

    if (!NIL_P(rb_timeout)) {
        timeout = time_from_rb(rb_timeout);
    }

    return BOOL2RB(sfSocketSelector_wait(Get_SocketSelector_Struct(self), timeout));
}

/* call-seq:
 *   tcp_listener_ready?(listener) -> true or false
 *
 * Call after #wait returns +true+ to check whether a given watched
 * TcpListener is one of the sockets that became ready.
 *
 * @return [Boolean]
 */
static VALUE SocketSelector_tcp_listener_ready(VALUE self, VALUE rb_socket) {
    return BOOL2RB(sfSocketSelector_isTcpListenerReady(Get_SocketSelector_Struct(self),
                                                       Get_TcpListener_Struct(rb_socket)));
}

/* call-seq:
 *   tcp_socket_ready?(socket) -> true or false
 *
 * Call after #wait returns +true+ to check whether a given watched
 * TcpSocket is one of the sockets that became ready.
 *
 * @return [Boolean]
 */
static VALUE SocketSelector_tcp_socket_ready(VALUE self, VALUE rb_socket) {
    return BOOL2RB(sfSocketSelector_isTcpSocketReady(Get_SocketSelector_Struct(self),
                                                     Get_TcpSocket_Struct(rb_socket)));
}

/* call-seq:
 *   udp_socket_ready?(socket) -> true or false
 *
 * Call after #wait returns +true+ to check whether a given watched
 * UdpSocket is one of the sockets that became ready.
 *
 * @return [Boolean]
 */
static VALUE SocketSelector_udp_socket_ready(VALUE self, VALUE rb_socket) {
    return BOOL2RB(sfSocketSelector_isUdpSocketReady(Get_SocketSelector_Struct(self),
                                                     Get_UdpSocket_Struct(rb_socket)));
}

/* Document-class: SF::Network::SocketSelector
 * Watches a set of TcpListener, TcpSocket and UdpSocket instances and blocks
 * until at least one of them is ready to read, so a single thread can
 * multiplex several sockets without polling.
 */
void Init_SocketSelector(VALUE rb_mNetwork) {
    rb_cSocketSelector = rb_define_class_under(rb_mNetwork, "SocketSelector", rb_cObject);
    rb_define_alloc_func(rb_cSocketSelector, SocketSelector_alloc);

    rb_define_method(rb_cSocketSelector, "initialize", SocketSelector_initialize, 0);

    rb_define_method(rb_cSocketSelector, "copy", SocketSelector_copy, 0);
    rb_define_method(rb_cSocketSelector, "add", SocketSelector_add, 1);
    rb_define_method(rb_cSocketSelector, "remove", SocketSelector_remove, 1);
    rb_define_method(rb_cSocketSelector, "clear", SocketSelector_clear, 0);
    rb_define_method(rb_cSocketSelector, "wait", SocketSelector_wait, -1);
    rb_define_method(rb_cSocketSelector, "tcp_listener_ready?", SocketSelector_tcp_listener_ready,
                     1);
    rb_define_method(rb_cSocketSelector, "tcp_socket_ready?", SocketSelector_tcp_socket_ready, 1);
    rb_define_method(rb_cSocketSelector, "udp_socket_ready?", SocketSelector_udp_socket_ready, 1);
}

VALUE Get_Klass_SocketSelector(void) {
    return rb_cSocketSelector;
}

void* Get_SocketSelector_Struct(VALUE self) {
    SocketSelector* ptr;
    TypedData_Get_Struct(self, SocketSelector, &SocketSelector_data_type, ptr);
    return ptr->handle;
}

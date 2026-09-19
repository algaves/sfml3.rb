#include "network/tcp_listener.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/macros.h"
#include "network/ip_address.h"
#include "network/network_enums.h"
#include "network/tcp_socket.h"

typedef struct {
    sfTcpListener* handle;
} TcpListener;

static VALUE rb_cTcpListener;

static void TcpListener_free(void* ptr) {
    TcpListener* listener = ptr;

    if (listener->handle != NULL) {
        sfTcpListener_destroy(listener->handle);
    }

    free(listener);
}

static const rb_data_type_t TcpListener_data_type = {
    .wrap_struct_name = "SFML::TcpListener",
    .function = {.dmark = NULL, .dfree = TcpListener_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE TcpListener_wrap(VALUE klass, sfTcpListener* handle) {
    TcpListener* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create TCP listener");
    }

    ptr = malloc(sizeof(TcpListener));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &TcpListener_data_type, ptr);
}

/* call-seq:
 *   TcpListener.new -> TcpListener
 *
 * Creates a new TCP listener that is not yet listening.
 *
 * @return [TcpListener]
 */
static VALUE TcpListener_new(VALUE klass) {
    return TcpListener_wrap(klass, sfTcpListener_create());
}

/* call-seq:
 *   TcpListener.any_port -> Integer
 *
 * Returns the port value that makes #listen bind to an OS-chosen port.
 *
 * @return [Integer] a port value that #listen interprets as "let the OS
 *   pick an available port"
 */
static VALUE TcpListener_any_port(VALUE klass) {
    return UINT2NUM(sfTcpListener_anyPort());
}

/* call-seq: blocking? -> true or false
 *
 * Returns +true+ if the listener is in blocking mode.
 *
 * @return [Boolean]
 */
static VALUE TcpListener_blocking(VALUE self) {
    return BOOL2RB(sfTcpListener_isBlocking(Get_TcpListener_Struct(self)));
}

/* call-seq:
 *   blocking=(value) -> value
 *
 * Enables or disables blocking mode according to +value+.
 *
 * @return [Boolean] +value+
 */
static VALUE TcpListener_set_blocking(VALUE self, VALUE rb_value) {
    sfTcpListener_setBlocking(Get_TcpListener_Struct(self), RTEST(rb_value));
    return rb_value;
}

/* call-seq: local_port -> Integer
 *
 * Returns the port the listener is bound to, or 0 if it isn't listening.
 *
 * @return [Integer] the port the listener is bound to, or 0 if it isn't
 *   listening
 */
static VALUE TcpListener_local_port(VALUE self) {
    return UINT2NUM(sfTcpListener_getLocalPort(Get_TcpListener_Struct(self)));
}

/* call-seq:
 *   listen(port, address = IpAddress::ANY) -> Symbol
 *
 * Starts listening for incoming connections on +port+, optionally
 * restricted to a specific local network interface via +address+.
 *
 * @return [Symbol] a SocketStatus name, +:done+ on success
 */
static VALUE TcpListener_listen(int argc, VALUE* argv, VALUE self) {
    VALUE rb_port, rb_address;

    rb_scan_args(argc, argv, "11", &rb_port, &rb_address);

    return ID2SYM(rb_intern(socket_status_name(
        sfTcpListener_listen(Get_TcpListener_Struct(self), (unsigned short)NUM2INT(rb_port),
                             ip_address_from_rb(rb_address, sfIpAddress_Any)))));
}

/* call-seq: close -> self
 *
 * Stops listening and closes the underlying socket.
 *
 * @return [self]
 */
static VALUE TcpListener_close(VALUE self) {
    sfTcpListener_close(Get_TcpListener_Struct(self));
    return self;
}

/* call-seq: accept -> [TcpSocket, Symbol]
 *
 * Blocks (unless #blocking? is +false+) waiting for a client to connect.
 *
 * @return [Array(TcpSocket, Symbol)] the connected socket (or +nil+ if the
 *   status isn't +:done+), and a SocketStatus name
 */
static VALUE TcpListener_accept(VALUE self) {
    sfTcpSocket* socket = NULL;
    sfSocketStatus status = sfTcpListener_accept(Get_TcpListener_Struct(self), &socket);
    VALUE rb_result = rb_ary_new_capa(2);

    rb_ary_push(rb_result, status == sfSocketDone ? tcp_socket_from_handle(socket) : Qnil);
    rb_ary_push(rb_result, ID2SYM(rb_intern(socket_status_name(status))));

    return rb_result;
}

/* Document-class: SFML::TcpListener
 * Listens for and accepts incoming TCP connections, producing connected
 * TcpSocket instances.
 */
void Init_TcpListener(VALUE rb_mSFML) {
    rb_cTcpListener = rb_define_class_under(rb_mSFML, "TcpListener", rb_cObject);

    rb_define_singleton_method(rb_cTcpListener, "new", TcpListener_new, 0);
    rb_define_singleton_method(rb_cTcpListener, "any_port", TcpListener_any_port, 0);

    rb_define_method(rb_cTcpListener, "blocking?", TcpListener_blocking, 0);
    rb_define_method(rb_cTcpListener, "blocking=", TcpListener_set_blocking, 1);
    rb_define_method(rb_cTcpListener, "local_port", TcpListener_local_port, 0);
    rb_define_method(rb_cTcpListener, "listen", TcpListener_listen, -1);
    rb_define_method(rb_cTcpListener, "close", TcpListener_close, 0);
    rb_define_method(rb_cTcpListener, "accept", TcpListener_accept, 0);
}

VALUE Get_Klass_TcpListener(void) {
    return rb_cTcpListener;
}

void* Get_TcpListener_Struct(VALUE self) {
    TcpListener* ptr;
    TypedData_Get_Struct(self, TcpListener, &TcpListener_data_type, ptr);
    return ptr->handle;
}

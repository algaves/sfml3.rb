#include "network/ip_address.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"
#include "system/time.h"

typedef struct {
    sfIpAddress address;
} IpAddress;

static VALUE rb_cIpAddress;

static void IpAddress_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t IpAddress_data_type = {
    .wrap_struct_name = "SF::Network::IpAddress",
    .function = {.dmark = NULL, .dfree = IpAddress_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

VALUE ip_address_to_rb(sfIpAddress address) {
    IpAddress* ptr = malloc(sizeof(IpAddress));

    ptr->address = address;

    return TypedData_Wrap_Struct(rb_cIpAddress, &IpAddress_data_type, ptr);
}

/* call-seq:
 *   IpAddress.from_string(address) -> IpAddress
 *
 * Builds an address from a string, which may be a dotted decimal
 * (+"192.168.1.1"+) or a network name (+"localhost"+).
 *
 * @return [IpAddress]
 */
static VALUE IpAddress_from_string(VALUE klass, VALUE rb_address) {
    return ip_address_to_rb(sfIpAddress_fromString(StringValueCStr(rb_address)));
}

/* call-seq:
 *   IpAddress.from_bytes(byte0, byte1, byte2, byte3) -> IpAddress
 *
 * Builds an address from its four decimal byte components (e.g.
 * +192, 168, 1, 1+).
 *
 * @return [IpAddress]
 */
static VALUE IpAddress_from_bytes(VALUE klass, VALUE rb0, VALUE rb1, VALUE rb2, VALUE rb3) {
    return ip_address_to_rb(sfIpAddress_fromBytes((uint8_t)NUM2INT(rb0), (uint8_t)NUM2INT(rb1),
                                                  (uint8_t)NUM2INT(rb2), (uint8_t)NUM2INT(rb3)));
}

/* call-seq:
 *   IpAddress.from_integer(value) -> IpAddress
 *
 * Builds an address from its 32-bit representation, in host byte order.
 *
 * @return [IpAddress]
 */
static VALUE IpAddress_from_integer(VALUE klass, VALUE rb_integer) {
    return ip_address_to_rb(sfIpAddress_fromInteger((uint32_t)NUM2UINT(rb_integer)));
}

static VALUE IpAddress_alloc(VALUE klass) {
    IpAddress* ptr = malloc(sizeof(IpAddress));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate IP address");
    }

    ptr->address = sfIpAddress_None;

    return TypedData_Wrap_Struct(klass, &IpAddress_data_type, ptr);
}

/* call-seq:
 *   IpAddress.new(address) -> IpAddress
 *
 * +address+ may be an Integer (32-bit representation), a String (dotted
 * decimal or network name), or another IpAddress to copy.
 *
 * @return [IpAddress]
 * @raise [TypeError] if +address+ is none of the above
 */
static VALUE IpAddress_initialize(VALUE self, VALUE rb_address) {
    IpAddress* ptr = (IpAddress*)Get_IpAddress_Struct(self);

    if (RB_INTEGER_TYPE_P(rb_address)) {
        ptr->address = sfIpAddress_fromInteger((uint32_t)NUM2UINT(rb_address));
    } else if (RB_TYPE_P(rb_address, T_STRING)) {
        ptr->address = sfIpAddress_fromString(StringValueCStr(rb_address));
    } else if (rb_obj_is_kind_of(rb_address, rb_cIpAddress)) {
        ptr->address = ((IpAddress*)Get_IpAddress_Struct(rb_address))->address;
    } else {
        raise_invalid_argument_class(rb_cIpAddress);
    }

    return self;
}

/* call-seq:
 *   IpAddress.local_address -> IpAddress
 *
 * Returns the address of the local computer on its local network.
 *
 * @return [IpAddress] the address of the local computer on the local
 *   network
 */
static VALUE IpAddress_local_address(VALUE klass) {
    return ip_address_to_rb(sfIpAddress_getLocalAddress());
}

/* call-seq:
 *   IpAddress.public_address(timeout = Time.zero) -> IpAddress
 *
 * Blocks while querying an external web service for the computer's
 * public address, as seen from outside the local network.
 *
 * @return [IpAddress] +IpAddress::NONE+ if the request timed out or failed
 */
static VALUE IpAddress_public_address(int argc, VALUE* argv, VALUE klass) {
    VALUE rb_timeout;
    sfTime timeout = sfTime_Zero;

    rb_scan_args(argc, argv, "01", &rb_timeout);

    if (!NIL_P(rb_timeout)) {
        timeout = time_from_rb(rb_timeout);
    }

    return ip_address_to_rb(sfIpAddress_getPublicAddress(timeout));
}

/* call-seq: to_s -> String
 *
 * Returns the address in dotted decimal notation.
 *
 * @return [String] dotted decimal representation, e.g. +"192.168.1.1"+
 */
static VALUE IpAddress_to_s(VALUE self) {
    char buffer[16];

    sfIpAddress_toString(((IpAddress*)Get_IpAddress_Struct(self))->address, buffer);

    return rb_str_new_cstr(buffer);
}

/* call-seq: to_integer -> Integer
 *
 * Returns the address as a 32-bit integer.
 *
 * @return [Integer] the 32-bit representation, in host byte order
 */
static VALUE IpAddress_to_integer(VALUE self) {
    return UINT2NUM(sfIpAddress_toInteger(((IpAddress*)Get_IpAddress_Struct(self))->address));
}

/* call-seq:
 *   self == other -> true or false
 *
 * Returns +true+ if +other+ is an IpAddress with the same value.
 *
 * @return [Boolean]
 */
static VALUE IpAddress_eql(VALUE self, VALUE rb_other) {
    sfIpAddress a = ((IpAddress*)Get_IpAddress_Struct(self))->address;
    sfIpAddress b;

    if (!rb_obj_is_kind_of(rb_other, rb_cIpAddress)) {
        return Qfalse;
    }

    b = ((IpAddress*)Get_IpAddress_Struct(rb_other))->address;

    return BOOL2RB(sfIpAddress_toInteger(a) == sfIpAddress_toInteger(b));
}

/* call-seq: hash -> Integer
 *
 * Returns a hash value consistent with #==, so addresses work as Hash keys.
 *
 * @return [Integer] a hash suitable for use as a Hash key, consistent with #==
 */
static VALUE IpAddress_hash(VALUE self) {
    return UINT2NUM(sfIpAddress_toInteger(((IpAddress*)Get_IpAddress_Struct(self))->address));
}

/* Document-class: SF::Network::IpAddress
 * An IPv4 network address.
 */
void Init_IpAddress(VALUE rb_mNetwork) {
    rb_cIpAddress = rb_define_class_under(rb_mNetwork, "IpAddress", rb_cObject);
    rb_define_alloc_func(rb_cIpAddress, IpAddress_alloc);

    rb_define_method(rb_cIpAddress, "initialize", IpAddress_initialize, 1);
    rb_define_singleton_method(rb_cIpAddress, "from_string", IpAddress_from_string, 1);
    rb_define_singleton_method(rb_cIpAddress, "from_bytes", IpAddress_from_bytes, 4);
    rb_define_singleton_method(rb_cIpAddress, "from_integer", IpAddress_from_integer, 1);
    rb_define_singleton_method(rb_cIpAddress, "local_address", IpAddress_local_address, 0);
    rb_define_singleton_method(rb_cIpAddress, "public_address", IpAddress_public_address, -1);

    rb_define_method(rb_cIpAddress, "to_s", IpAddress_to_s, 0);
    rb_define_method(rb_cIpAddress, "to_str", IpAddress_to_s, 0);
    rb_define_method(rb_cIpAddress, "to_integer", IpAddress_to_integer, 0);
    rb_define_method(rb_cIpAddress, "hash", IpAddress_hash, 0);
    rb_define_method(rb_cIpAddress, "==", IpAddress_eql, 1);
    rb_define_method(rb_cIpAddress, "eql?", IpAddress_eql, 1);

    /* An invalid/unspecified address. */
    rb_define_const(rb_cIpAddress, "NONE", rb_obj_freeze(ip_address_to_rb(sfIpAddress_None)));
    /* Any address, 0.0.0.0, e.g. to bind a listener to all network interfaces. */
    rb_define_const(rb_cIpAddress, "ANY", rb_obj_freeze(ip_address_to_rb(sfIpAddress_Any)));
    /* The local host address, 127.0.0.1. */
    rb_define_const(rb_cIpAddress, "LOCAL_HOST",
                    rb_obj_freeze(ip_address_to_rb(sfIpAddress_LocalHost)));
    /* The broadcast address, 255.255.255.255. */
    rb_define_const(rb_cIpAddress, "BROADCAST",
                    rb_obj_freeze(ip_address_to_rb(sfIpAddress_Broadcast)));
}

VALUE Get_Klass_IpAddress(void) {
    return rb_cIpAddress;
}

void* Get_IpAddress_Struct(VALUE self) {
    IpAddress* ptr;
    TypedData_Get_Struct(self, IpAddress, &IpAddress_data_type, ptr);
    return ptr;
}

sfIpAddress ip_address_from_rb(VALUE rb_address, sfIpAddress fallback) {
    if (NIL_P(rb_address)) {
        return fallback;
    }

    if (rb_obj_is_kind_of(rb_address, rb_cIpAddress)) {
        return ((IpAddress*)Get_IpAddress_Struct(rb_address))->address;
    }

    if (RB_TYPE_P(rb_address, T_STRING)) {
        return sfIpAddress_fromString(StringValueCStr(rb_address));
    }

    if (RB_INTEGER_TYPE_P(rb_address)) {
        return sfIpAddress_fromInteger((uint32_t)NUM2UINT(rb_address));
    }

    raise_invalid_argument_class(rb_cIpAddress);

    return fallback;
}

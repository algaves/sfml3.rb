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

static void IpAddress_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t IpAddress_data_type = {
    .wrap_struct_name = "SFML::IpAddress",
    .function = {.dmark = NULL, .dfree = IpAddress_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

VALUE ip_address_to_rb(sfIpAddress address) {
    IpAddress *ptr = malloc(sizeof(IpAddress));

    ptr->address = address;

    return TypedData_Wrap_Struct(rb_cIpAddress, &IpAddress_data_type, ptr);
}

static VALUE IpAddress_from_string(VALUE klass, VALUE rb_address) {
    return ip_address_to_rb(sfIpAddress_fromString(StringValueCStr(rb_address)));
}

static VALUE IpAddress_from_bytes(VALUE klass, VALUE rb0, VALUE rb1, VALUE rb2, VALUE rb3) {
    return ip_address_to_rb(sfIpAddress_fromBytes((uint8_t) NUM2INT(rb0), (uint8_t) NUM2INT(rb1),
                                                  (uint8_t) NUM2INT(rb2), (uint8_t) NUM2INT(rb3)));
}

static VALUE IpAddress_from_integer(VALUE klass, VALUE rb_integer) {
    return ip_address_to_rb(sfIpAddress_fromInteger((uint32_t) NUM2UINT(rb_integer)));
}

static VALUE IpAddress_new(VALUE klass, VALUE rb_address) {
    if (RB_INTEGER_TYPE_P(rb_address)) {
        return IpAddress_from_integer(klass, rb_address);
    }

    if (RB_TYPE_P(rb_address, T_STRING)) {
        return IpAddress_from_string(klass, rb_address);
    }

    if (rb_obj_is_kind_of(rb_address, rb_cIpAddress)) {
        return ip_address_to_rb(((IpAddress *) Get_IpAddress_Struct(rb_address))->address);
    }

    raise_invalid_argument_class(rb_cIpAddress);

    return Qnil;
}

static VALUE IpAddress_local_address(VALUE klass) {
    return ip_address_to_rb(sfIpAddress_getLocalAddress());
}

static VALUE IpAddress_public_address(int argc, VALUE *argv, VALUE klass) {
    VALUE rb_timeout;
    sfTime timeout = sfTime_Zero;

    rb_scan_args(argc, argv, "01", &rb_timeout);

    if (!NIL_P(rb_timeout)) {
        timeout = time_from_rb(rb_timeout);
    }

    return ip_address_to_rb(sfIpAddress_getPublicAddress(timeout));
}

static VALUE IpAddress_to_s(VALUE self) {
    char buffer[16];

    sfIpAddress_toString(((IpAddress *) Get_IpAddress_Struct(self))->address, buffer);

    return rb_str_new_cstr(buffer);
}

static VALUE IpAddress_to_integer(VALUE self) {
    return UINT2NUM(sfIpAddress_toInteger(((IpAddress *) Get_IpAddress_Struct(self))->address));
}

static VALUE IpAddress_eql(VALUE self, VALUE rb_other) {
    sfIpAddress a = ((IpAddress *) Get_IpAddress_Struct(self))->address;
    sfIpAddress b;

    if (!rb_obj_is_kind_of(rb_other, rb_cIpAddress)) {
        return Qfalse;
    }

    b = ((IpAddress *) Get_IpAddress_Struct(rb_other))->address;

    return BOOL2RB(sfIpAddress_toInteger(a) == sfIpAddress_toInteger(b));
}

static VALUE IpAddress_hash(VALUE self) {
    return UINT2NUM(sfIpAddress_toInteger(((IpAddress *) Get_IpAddress_Struct(self))->address));
}

void Init_IpAddress(VALUE rb_module) {
    rb_cIpAddress = rb_define_class_under(rb_module, "IpAddress", rb_cObject);

    rb_define_singleton_method(rb_cIpAddress, "new", IpAddress_new, 1);
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

    rb_define_const(rb_cIpAddress, "NONE", ip_address_to_rb(sfIpAddress_None));
    rb_define_const(rb_cIpAddress, "ANY", ip_address_to_rb(sfIpAddress_Any));
    rb_define_const(rb_cIpAddress, "LOCAL_HOST", ip_address_to_rb(sfIpAddress_LocalHost));
    rb_define_const(rb_cIpAddress, "BROADCAST", ip_address_to_rb(sfIpAddress_Broadcast));
}

VALUE Get_Klass_IpAddress(void) {
    return rb_cIpAddress;
}

void *Get_IpAddress_Struct(VALUE self) {
    IpAddress *ptr;
    TypedData_Get_Struct(self, IpAddress, &IpAddress_data_type, ptr);
    return ptr;
}

sfIpAddress ip_address_from_rb(VALUE rb_address, sfIpAddress fallback) {
    if (NIL_P(rb_address)) {
        return fallback;
    }

    if (rb_obj_is_kind_of(rb_address, rb_cIpAddress)) {
        return ((IpAddress *) Get_IpAddress_Struct(rb_address))->address;
    }

    if (RB_TYPE_P(rb_address, T_STRING)) {
        return sfIpAddress_fromString(StringValueCStr(rb_address));
    }

    if (RB_INTEGER_TYPE_P(rb_address)) {
        return sfIpAddress_fromInteger((uint32_t) NUM2UINT(rb_address));
    }

    raise_invalid_argument_class(rb_cIpAddress);

    return fallback;
}

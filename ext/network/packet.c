#include "network/packet.h"

#include <ruby.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfPacket* packet;
} Packet;

static VALUE rb_cPacket;

static void Packet_free(void* ptr) {
    Packet* packet = ptr;

    sfPacket_destroy(packet->packet);
    free(packet);
}

static const rb_data_type_t Packet_data_type = {
    .wrap_struct_name = "SFML::Packet",
    .function = {.dmark = NULL, .dfree = Packet_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Packet_wrap(VALUE klass, sfPacket* packet) {
    Packet* ptr;

    if (packet == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create packet");
    }

    ptr = malloc(sizeof(Packet));
    ptr->packet = packet;

    return TypedData_Wrap_Struct(klass, &Packet_data_type, ptr);
}

static VALUE Packet_alloc(VALUE klass) {
    return Packet_wrap(klass, sfPacket_create());
}

/* call-seq:
 *   Packet.new -> Packet
 *
 * Creates a new, empty packet.
 *
 * @return [Packet] a new, empty packet
 */
static VALUE Packet_initialize(VALUE self) {
    return self;
}

/* call-seq: copy -> Packet
 *
 * Returns an independent copy of the packet.
 *
 * @return [Packet] an independent copy with the same data and read position
 */
static VALUE Packet_copy(VALUE self) {
    return Packet_wrap(Get_Klass_Packet(), sfPacket_copy(Get_Packet_Struct(self)));
}

/* call-seq:
 *   append(data) -> self
 *
 * Appends raw bytes to the end of the packet, growing it as needed.
 * +data+ is any String-convertible value.
 *
 * @return [self]
 */
static VALUE Packet_append(VALUE self, VALUE rb_data) {
    StringValue(rb_data);

    sfPacket_append(Get_Packet_Struct(self), RSTRING_PTR(rb_data), (size_t)RSTRING_LEN(rb_data));

    return self;
}

/* call-seq: clear -> self
 *
 * Empties the packet and resets its read position to zero.
 *
 * @return [self]
 */
static VALUE Packet_clear(VALUE self) {
    sfPacket_clear(Get_Packet_Struct(self));
    return self;
}

/* call-seq: data -> String
 *
 * Returns the packet's raw byte content.
 *
 * @return [String] the packet's raw byte content
 */
static VALUE Packet_data(VALUE self) {
    const void* data = sfPacket_getData(Get_Packet_Struct(self));

    return rb_str_new((const char*)data, (long)sfPacket_getDataSize(Get_Packet_Struct(self)));
}

/* call-seq: data_size -> Integer
 *
 * Returns the number of bytes held by the packet.
 *
 * @return [Integer] the number of bytes in the packet's data
 */
static VALUE Packet_data_size(VALUE self) {
    return SIZET2NUM(sfPacket_getDataSize(Get_Packet_Struct(self)));
}

/* call-seq: read_position -> Integer
 *
 * Returns the current read position, in bytes.
 *
 * @return [Integer] the current read position, in bytes
 */
static VALUE Packet_read_position(VALUE self) {
    return SIZET2NUM(sfPacket_getReadPosition(Get_Packet_Struct(self)));
}

/* call-seq: end_of_packet? -> true or false
 *
 * Returns +true+ once all of the packet's data has been read.
 *
 * @return [Boolean] whether the end of the packet has been reached, i.e.
 *   whether all the data has been read
 */
static VALUE Packet_end_of_packet(VALUE self) {
    return BOOL2RB(sfPacket_endOfPacket(Get_Packet_Struct(self)));
}

/* call-seq: can_read? -> true or false
 *
 * Returns +true+ if the packet is still in a valid reading state.
 *
 * @return [Boolean] whether the packet is in a valid reading state, i.e.
 *   whether the last read operation succeeded
 */
static VALUE Packet_can_read(VALUE self) {
    return BOOL2RB(sfPacket_canRead(Get_Packet_Struct(self)));
}

#define PACKET_READER(name, c_type, conv)                                                          \
    static VALUE Packet_read_##name(VALUE self) {                                                  \
        return conv(sfPacket_read##c_type(Get_Packet_Struct(self)));                               \
    }

#define PACKET_WRITER(name, c_type, num2)                                                          \
    static VALUE Packet_write_##name(VALUE self, VALUE rb_value) {                                 \
        sfPacket_write##c_type(Get_Packet_Struct(self), num2(rb_value));                           \
        return rb_value;                                                                           \
    }

PACKET_READER(bool, Bool, BOOL2RB)
PACKET_READER(int8, Int8, INT2NUM)
PACKET_READER(uint8, Uint8, UINT2NUM)
PACKET_READER(int16, Int16, INT2NUM)
PACKET_READER(uint16, Uint16, UINT2NUM)
PACKET_READER(int32, Int32, INT2NUM)
PACKET_READER(uint32, Uint32, UINT2NUM)
PACKET_READER(int64, Int64, LL2NUM)
PACKET_READER(uint64, Uint64, ULL2NUM)
PACKET_READER(float, Float, DBL2NUM)
PACKET_READER(double, Double, DBL2NUM)

PACKET_WRITER(bool, Bool, RTEST)
PACKET_WRITER(int8, Int8, (int8_t)NUM2INT)
PACKET_WRITER(uint8, Uint8, (uint8_t)NUM2INT)
PACKET_WRITER(int16, Int16, (int16_t)NUM2INT)
PACKET_WRITER(uint16, Uint16, (uint16_t)NUM2INT)
PACKET_WRITER(int32, Int32, (int32_t)NUM2INT)
PACKET_WRITER(uint32, Uint32, (uint32_t)NUM2UINT)
PACKET_WRITER(int64, Int64, (int64_t)NUM2LL)
PACKET_WRITER(uint64, Uint64, (uint64_t)NUM2ULL)
PACKET_WRITER(float, Float, (float)NUM2DBL)
PACKET_WRITER(double, Double, NUM2DBL)

/* Document-method: SFML::Packet#read_string
 * call-seq: read_string -> String
 *
 * Reads a NUL-terminated string previously written with #write_string.
 *
 * @return [String]
 */
static VALUE Packet_read_string(VALUE self) {
    void* packet = Get_Packet_Struct(self);
    /* sfPacket_readString has no destination-length parameter: CSFML's C++
       side bounds the string it extracts by the packet's own remaining
       unread data, but then copies that (unbounded, from the caller's point
       of view) string into whatever buffer it's given -- a fixed-size stack
       buffer here would be a stack overflow for a packet holding a longer
       string (e.g. one received from the network). The extracted string can
       never exceed the packet's remaining data size, so sizing the buffer to
       that is always sufficient. */
    size_t remaining = sfPacket_getDataSize(packet) - sfPacket_getReadPosition(packet);
    char* buffer = malloc(remaining + 1);
    VALUE result;

    if (buffer == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate read buffer");
    }

    buffer[0] = '\0';
    sfPacket_readString(packet, buffer);
    buffer[remaining] = '\0';

    result = rb_str_new_cstr(buffer);
    free(buffer);

    return result;
}

/* call-seq:
 *   write_string(value) -> value
 *
 * Writes a NUL-terminated string, readable back with #read_string.
 *
 * @return [String] +value+
 */
static VALUE Packet_write_string(VALUE self, VALUE rb_value) {
    sfPacket_writeString(Get_Packet_Struct(self), StringValueCStr(rb_value));
    return rb_value;
}

/* Document-class: SFML::Packet
 * A structured byte buffer for use with TcpSocket/UdpSocket, with typed
 * sequential read/write access. Reads and writes must happen in the same
 * order the data was written, since the packet keeps no type tags on the
 * wire.
 */
void Init_Packet(VALUE rb_mSFML) {
    rb_cPacket = rb_define_class_under(rb_mSFML, "Packet", rb_cObject);
    rb_define_alloc_func(rb_cPacket, Packet_alloc);

    rb_define_method(rb_cPacket, "initialize", Packet_initialize, 0);

    rb_define_method(rb_cPacket, "copy", Packet_copy, 0);
    rb_define_method(rb_cPacket, "append", Packet_append, 1);
    rb_define_method(rb_cPacket, "clear", Packet_clear, 0);
    rb_define_method(rb_cPacket, "data", Packet_data, 0);
    rb_define_method(rb_cPacket, "data_size", Packet_data_size, 0);
    rb_define_method(rb_cPacket, "read_position", Packet_read_position, 0);
    rb_define_method(rb_cPacket, "end_of_packet?", Packet_end_of_packet, 0);
    rb_define_method(rb_cPacket, "can_read?", Packet_can_read, 0);

    /* call-seq: read_bool -> true or false
     *
     * Reads a boolean from the packet.
     *
     * @return [Boolean]
     */
    rb_define_method(rb_cPacket, "read_bool", Packet_read_bool, 0);
    /* call-seq: read_int8 -> Integer
     *
     * Reads a signed 8-bit integer from the packet.
     *
     * @return [Integer] a signed 8-bit integer
     */
    rb_define_method(rb_cPacket, "read_int8", Packet_read_int8, 0);
    /* call-seq: read_uint8 -> Integer
     *
     * Reads an unsigned 8-bit integer from the packet.
     *
     * @return [Integer] an unsigned 8-bit integer
     */
    rb_define_method(rb_cPacket, "read_uint8", Packet_read_uint8, 0);
    /* call-seq: read_int16 -> Integer
     *
     * Reads a signed 16-bit integer from the packet.
     *
     * @return [Integer] a signed 16-bit integer
     */
    rb_define_method(rb_cPacket, "read_int16", Packet_read_int16, 0);
    /* call-seq: read_uint16 -> Integer
     *
     * Reads an unsigned 16-bit integer from the packet.
     *
     * @return [Integer] an unsigned 16-bit integer
     */
    rb_define_method(rb_cPacket, "read_uint16", Packet_read_uint16, 0);
    /* call-seq: read_int32 -> Integer
     *
     * Reads a signed 32-bit integer from the packet.
     *
     * @return [Integer] a signed 32-bit integer
     */
    rb_define_method(rb_cPacket, "read_int32", Packet_read_int32, 0);
    /* call-seq: read_uint32 -> Integer
     *
     * Reads an unsigned 32-bit integer from the packet.
     *
     * @return [Integer] an unsigned 32-bit integer
     */
    rb_define_method(rb_cPacket, "read_uint32", Packet_read_uint32, 0);
    /* call-seq: read_int64 -> Integer
     *
     * Reads a signed 64-bit integer from the packet.
     *
     * @return [Integer] a signed 64-bit integer
     */
    rb_define_method(rb_cPacket, "read_int64", Packet_read_int64, 0);
    /* call-seq: read_uint64 -> Integer
     *
     * Reads an unsigned 64-bit integer from the packet.
     *
     * @return [Integer] an unsigned 64-bit integer
     */
    rb_define_method(rb_cPacket, "read_uint64", Packet_read_uint64, 0);
    /* call-seq: read_float -> Float
     *
     * Reads a single-precision float from the packet.
     *
     * @return [Float] a single-precision float
     */
    rb_define_method(rb_cPacket, "read_float", Packet_read_float, 0);
    /* call-seq: read_double -> Float
     *
     * Reads a double-precision float from the packet.
     *
     * @return [Float] a double-precision float
     */
    rb_define_method(rb_cPacket, "read_double", Packet_read_double, 0);
    rb_define_method(rb_cPacket, "read_string", Packet_read_string, 0);

    /* call-seq:
     *   write_bool(value) -> value
     *
     * Writes +value+ as a boolean.
     *
     * @return [Boolean] +value+
     */
    rb_define_method(rb_cPacket, "write_bool", Packet_write_bool, 1);
    /* call-seq:
     *   write_int8(value) -> value
     *
     * Writes +value+ as a signed 8-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_int8", Packet_write_int8, 1);
    /* call-seq:
     *   write_uint8(value) -> value
     *
     * Writes +value+ as an unsigned 8-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_uint8", Packet_write_uint8, 1);
    /* call-seq:
     *   write_int16(value) -> value
     *
     * Writes +value+ as a signed 16-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_int16", Packet_write_int16, 1);
    /* call-seq:
     *   write_uint16(value) -> value
     *
     * Writes +value+ as an unsigned 16-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_uint16", Packet_write_uint16, 1);
    /* call-seq:
     *   write_int32(value) -> value
     *
     * Writes +value+ as a signed 32-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_int32", Packet_write_int32, 1);
    /* call-seq:
     *   write_uint32(value) -> value
     *
     * Writes +value+ as an unsigned 32-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_uint32", Packet_write_uint32, 1);
    /* call-seq:
     *   write_int64(value) -> value
     *
     * Writes +value+ as a signed 64-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_int64", Packet_write_int64, 1);
    /* call-seq:
     *   write_uint64(value) -> value
     *
     * Writes +value+ as an unsigned 64-bit integer.
     *
     * @return [Integer] +value+
     */
    rb_define_method(rb_cPacket, "write_uint64", Packet_write_uint64, 1);
    /* call-seq:
     *   write_float(value) -> value
     *
     * Writes +value+ as a single-precision float.
     *
     * @return [Float] +value+
     */
    rb_define_method(rb_cPacket, "write_float", Packet_write_float, 1);
    /* call-seq:
     *   write_double(value) -> value
     *
     * Writes +value+ as a double-precision float.
     *
     * @return [Float] +value+
     */
    rb_define_method(rb_cPacket, "write_double", Packet_write_double, 1);
    rb_define_method(rb_cPacket, "write_string", Packet_write_string, 1);
}

VALUE Get_Klass_Packet(void) {
    return rb_cPacket;
}

void* Get_Packet_Struct(VALUE self) {
    Packet* ptr;
    TypedData_Get_Struct(self, Packet, &Packet_data_type, ptr);
    return ptr->packet;
}

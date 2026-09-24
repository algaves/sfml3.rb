# frozen_string_literal: true

# A Packet is a typed, self-describing byte buffer: you write values in order,
# send the whole packet, and the other side reads them back in the same order.
# It is how SFML turns game state into bytes without hand-rolling packing. This
# script writes every supported type, reads them back, and shows the cursor and
# end-of-packet helpers. Console only, no window.
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/network/packets/packets.rb

require 'sfml'
include SF::Network

def rule(title)
  puts "\n== #{title}"
end

rule 'Write every type, in order'
out = Packet.new
out.write_bool(true)
out.write_int8(-8)
out.write_uint8(200)
out.write_int16(-1_600)
out.write_uint16(60_000)
out.write_int32(-100_000)
out.write_uint32(3_000_000_000)
out.write_int64(-9_000_000_000)
out.write_uint64(18_000_000_000)
out.write_float(1.5)
out.write_double(2.25)
out.write_string('hello packets')
puts "packet size after writes -> #{out.data_size} bytes"
puts "cursor at the end        -> read_position #{out.read_position}, end_of_packet? #{out.end_of_packet?}"

rule 'Read them back in the same order'
inbound = Packet.new
inbound.append(out.data)
puts "bool   #{inbound.read_bool}"
puts "int8   #{inbound.read_int8}"
puts "uint8  #{inbound.read_uint8}"
puts "int16  #{inbound.read_int16}"
puts "uint16 #{inbound.read_uint16}"
puts "int32  #{inbound.read_int32}"
puts "uint32 #{inbound.read_uint32}"
puts "int64  #{inbound.read_int64}"
puts "uint64 #{inbound.read_uint64}"
puts "float  #{inbound.read_float}"
puts "double #{inbound.read_double}"
puts "string #{inbound.read_string.inspect}"
puts "end_of_packet? -> #{inbound.end_of_packet?}"

rule 'A small message protocol'
# Prefix each message with a type tag, then its fields: a pattern that scales
# to a real protocol. Both sides must agree on the layout.
def player_update(id, x, y, health)
  packet = Packet.new
  packet.write_uint8(1) # tag 1 = player update
  packet.write_uint32(id)
  packet.write_float(x)
  packet.write_float(y)
  packet.write_uint16(health)
  packet
end

def read_message(packet)
  case packet.read_uint8
  when 1
    { type: :player_update, id: packet.read_uint32, x: packet.read_float,
      y: packet.read_float, health: packet.read_uint16 }
  else
    { type: :unknown }
  end
end

message = player_update(42, 128.0, 64.5, 90)
puts "encoded #{message.data_size} bytes -> #{read_message(message).inspect}"

rule 'Rules of thumb'
puts '- Reads must mirror the writes exactly, in order and in type.'
puts '- write_* returns self, so a packet can be built by chaining.'
puts '- append/clear! let you reuse one buffer; copy duplicates a packet.'

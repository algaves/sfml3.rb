# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Packet do
  it 'round-trips every packet field type' do
    packet = SFML::Packet.new
    packet.write_bool(true)
    packet.write_int8(-8)
    packet.write_uint8(200)
    packet.write_int16(-32_000)
    packet.write_uint16(60_000)
    packet.write_int32(-2_000_000_000)
    packet.write_uint32(4_000_000_000)
    packet.write_int64(-9_000_000_000)
    packet.write_uint64(18_000_000_000)
    packet.write_float(1.5)
    packet.write_double(2.5)
    packet.write_string('packet payload')

    copy = packet.copy

    expect(copy.read_bool).to eq(true)
    expect(copy.read_int8).to eq(-8)
    expect(copy.read_uint8).to eq(200)
    expect(copy.read_int16).to eq(-32_000)
    expect(copy.read_uint16).to eq(60_000)
    expect(copy.read_int32).to eq(-2_000_000_000)
    expect(copy.read_uint32).to eq(4_000_000_000)
    expect(copy.read_int64).to eq(-9_000_000_000)
    expect(copy.read_uint64).to eq(18_000_000_000)
    expect(copy.read_float).to be_within(0.0001).of(1.5)
    expect(copy.read_double).to be_within(0.0001).of(2.5)
    expect(copy.read_string).to eq('packet payload')
  end

  it 'exposes raw data and supports append' do
    packet = SFML::Packet.new
    packet.append('abcd')
    packet.append("\x00\x01")

    expect(packet.data_size).to eq(6)
    expect(packet.data.b).to eq("abcd\x00\x01".b)
  end
end

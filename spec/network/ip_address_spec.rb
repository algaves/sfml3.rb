# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::IpAddress do
  it 'round-trips an IP address' do
    address = SFML::IpAddress.from_string('127.0.0.1')

    expect(address.to_s).to eq('127.0.0.1')
    expect(address.to_integer).to eq(2_130_706_433)
    expect(SFML::IpAddress.from_bytes(127, 0, 0, 1)).to eq(address)
    expect(SFML::IpAddress.from_integer(address.to_integer)).to eq(address)
    expect(SFML::IpAddress::ANY).not_to eq(address)
  end

  it 'exposes the predefined addresses' do
    expect(SFML::IpAddress::ANY.to_s).to eq('0.0.0.0')
    expect(SFML::IpAddress::BROADCAST.to_s).to eq('255.255.255.255')
    expect(SFML::IpAddress::LOCAL_HOST.to_s).to eq('127.0.0.1')
    expect(SFML::IpAddress.local_address).to be_a(SFML::IpAddress)
  end
end

# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Network::IpAddress do
  it 'round-trips an IP address' do
    address = SF::Network::IpAddress.from_string('127.0.0.1')

    expect(address.to_s).to eq('127.0.0.1')
    expect(address.to_integer).to eq(2_130_706_433)
    expect(SF::Network::IpAddress.from_bytes(127, 0, 0, 1)).to eq(address)
    expect(SF::Network::IpAddress.from_integer(address.to_integer)).to eq(address)
    expect(SF::Network::IpAddress::ANY).not_to eq(address)
  end

  it 'exposes the predefined addresses' do
    expect(SF::Network::IpAddress::ANY.to_s).to eq('0.0.0.0')
    expect(SF::Network::IpAddress::BROADCAST.to_s).to eq('255.255.255.255')
    expect(SF::Network::IpAddress::LOCAL_HOST.to_s).to eq('127.0.0.1')
    expect(SF::Network::IpAddress.local_address).to be_a(SF::Network::IpAddress)
  end
end

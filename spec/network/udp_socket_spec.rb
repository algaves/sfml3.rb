# frozen_string_literal: true

require_relative '../spec_helper'
require 'timeout'

RSpec.describe SF::Network::UdpSocket do
  it 'talks UDP over loopback' do
    socket = SF::Network::UdpSocket.new
    expect(socket.bind(0)).to eq(:done)

    port = socket.local_port
    expect(socket.send('pong', '127.0.0.1', port)).to eq(:done)

    data, address, remote_port, status = Timeout.timeout(5) { socket.receive(64) }

    expect(status).to eq(:done)
    expect(data).to eq('pong')
    expect(address.to_s).to eq('127.0.0.1')
    expect(remote_port).to eq(port)
  end
end

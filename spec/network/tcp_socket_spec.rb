# frozen_string_literal: true

require_relative '../spec_helper'
require 'timeout'

RSpec.describe SF::Network::TcpSocket do
  it 'exposes the socket status constants' do
    expect(SF::Network::SocketStatus::DONE).to eq(0)
    expect(SF::Network::SocketStatus::ERROR).to eq(4)
  end

  it 'talks TCP over loopback' do
    listener = SF::Network::TcpListener.new
    expect(listener.listen(0)).to eq(:done)

    client = SF::Network::TcpSocket.new
    expect(Timeout.timeout(5) { client.connect('127.0.0.1', listener.local_port) }).to eq(:done)

    connection, status = Timeout.timeout(5) { listener.accept }
    expect(status).to eq(:done)
    expect(connection).to be_a(SF::Network::TcpSocket)

    expect(client.send('ping')).to eq(:done)
    data, receive_status = Timeout.timeout(5) { connection.receive(64) }

    expect(receive_status).to eq(:done)
    expect(data).to eq('ping')
  end
end

# frozen_string_literal: true

require_relative '../spec_helper'
require 'timeout'

RSpec.describe SF::Network::SocketSelector do
  it 'waits on a selector for a connection' do
    listener = SF::Network::TcpListener.new
    listener.listen(0)

    selector = SF::Network::SocketSelector.new
    selector.add(listener)

    # A zero timeout means "block forever" in SFML, so an explicit tiny
    # timeout is what expresses a non-blocking poll.
    expect(selector.wait(SF::System::Time.microseconds(1))).to eq(false)

    client = SF::Network::TcpSocket.new
    Timeout.timeout(5) { client.connect('127.0.0.1', listener.local_port) }

    expect(Timeout.timeout(5) { selector.wait(SF::System::Time.seconds(2)) }).to be(true)
    expect(selector).to be_tcp_listener_ready(listener)

    selector.remove(listener)
    selector.clear
  end
end

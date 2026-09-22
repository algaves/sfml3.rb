# frozen_string_literal: true

require_relative '../spec_helper'
require 'timeout'

RSpec.describe SFML::SocketSelector do
  it 'waits on a selector for a connection' do
    listener = SFML::TcpListener.new
    listener.listen(0)

    selector = SFML::SocketSelector.new
    selector.add(listener)

    # A zero timeout means "block forever" in SFML, so an explicit tiny
    # timeout is what expresses a non-blocking poll.
    expect(selector.wait(SFML::Time.microseconds(1))).to eq(false)

    client = SFML::TcpSocket.new
    Timeout.timeout(5) { client.connect('127.0.0.1', listener.local_port) }

    expect(Timeout.timeout(5) { selector.wait(SFML::Time.seconds(2)) }).to be(true)
    expect(selector).to be_tcp_listener_ready(listener)

    selector.remove(listener)
    selector.clear
  end
end

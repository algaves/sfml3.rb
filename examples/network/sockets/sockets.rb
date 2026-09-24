# frozen_string_literal: true

# Sockets and addresses, without a window. An IpAddress is a 32-bit address you
# can build from a string, four bytes or an integer; every network call returns
# a SocketStatus symbol (:done on success) and sockets can also be switched to
# non-blocking mode. This script parses addresses, prints the constants, and
# resolves this machine's local and public addresses (best effort).
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/network/sockets/sockets.rb

require 'sfml'
include SF::Network
include SF::System

def rule(title)
  puts "\n== #{title}"
end

rule 'Building and printing addresses'
from_string = IpAddress.from_string('127.0.0.1')
from_bytes = IpAddress.from_bytes(127, 0, 0, 1)
from_integer = IpAddress.from_integer(2_130_706_433) # 0x7F000001
puts "IpAddress.from_string('127.0.0.1') -> #{from_string}"
puts "IpAddress.from_bytes(127, 0, 0, 1)  -> #{from_bytes}"
puts "IpAddress.from_integer(0x7F000001)  -> #{from_integer}"
puts "to_integer                          -> #{from_string.to_integer} (0x#{from_string.to_integer.to_s(16).upcase})"
puts "all three compare equal             -> #{from_string == from_bytes && from_bytes == from_integer}"

rule 'The named constants'
[IpAddress::NONE, IpAddress::ANY, IpAddress::LOCAL_HOST, IpAddress::BROADCAST].each do |address|
  name = address.to_s
  puts "#{name.ljust(10)} = #{name.ljust(15)} (#{address.to_integer})"
end

rule 'Local address (OS resolver)'
started = Clock.new
begin
  local = IpAddress.local_address
  puts "IpAddress.local_address -> #{local} (#{started.elapsed_time.as_seconds.round(3)}s)"
rescue StandardError => e
  puts "IpAddress.local_address failed: #{e.class}: #{e.message}"
end

rule 'Public address (needs the internet, times out cleanly)'
started = Clock.new
begin
  public_address = IpAddress.public_address(Time.seconds(2.0))
  puts "IpAddress.public_address -> #{public_address} (#{started.elapsed_time.as_seconds.round(3)}s)"
rescue StandardError => e
  puts "IpAddress.public_address failed: #{e.class}: #{e.message}"
end

rule 'SocketStatus: every call tells you what happened'
# Connecting to a port nothing listens on returns :error quickly on loopback.
begin
  socket = TcpSocket.new
  status = socket.connect(IpAddress::LOCAL_HOST, 1, Time.seconds(0.5))
  puts "connect to an unused port -> #{status.inspect} (expected :error)"
  socket.disconnect
rescue StandardError => e
  puts "connect raised: #{e.class}: #{e.message}"
end

rule 'Blocking vs non-blocking'
socket = TcpSocket.new
puts "blocking? -> #{socket.blocking?}"
socket.blocking = false
puts "after blocking = false, blocking? -> #{socket.blocking?}"
puts 'A non-blocking receive returns at once with ["", :not_ready] instead of waiting.'

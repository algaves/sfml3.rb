# frozen_string_literal: true

# Name resolution without a window. SFML 3.1 added a `Dns` class for MX/SRV
# lookups, but this gem vendors CSFML 3.0.0 (which predates it), so the bound
# DNS surface is SF::Network::IpAddress: `local_address` and `public_address(timeout)`
# resolve through the OS resolver, `from_string` / `from_bytes` /
# `from_integer` build addresses locally, and a TcpSocket connect resolves a
# hostname as a side effect. Network lookups are best-effort and time out
# cleanly when offline.
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/subsystems/dns.rb

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

def rule(title)
  puts "\n== #{title}"
end

rule 'Local address'
puts "IpAddress.local_address       -> #{IpAddress.local_address}"
puts "IpAddress::LOCAL_HOST         -> #{IpAddress::LOCAL_HOST}"
puts "IpAddress::ANY                -> #{IpAddress::ANY}"
puts "IpAddress::BROADCAST          -> #{IpAddress::BROADCAST}"
puts "IpAddress::NONE               -> #{IpAddress::NONE} (#{IpAddress::NONE.to_integer})"

rule 'Parsing and round-trips (no network)'
from_string = IpAddress.from_string('192.168.1.42')
from_bytes = IpAddress.from_bytes(192, 168, 1, 42)
from_integer = IpAddress.from_integer(from_bytes.to_integer)
puts "from_string('192.168.1.42')   -> #{from_string}"
puts "from_bytes(192, 168, 1, 42)   -> #{from_bytes}  (#{from_bytes.to_integer})"
puts "from_integer(#{from_bytes.to_integer})    -> #{from_integer}"
puts 'all equal?                    -> ' \
     "#{from_string == from_bytes && from_bytes == from_integer}"

rule 'Public address (needs the network)'
begin
  public = IpAddress.public_address(SF::System::Time.seconds(3))
  if public == IpAddress::NONE
    puts 'public_address timed out or is unavailable'
  else
    puts "IpAddress.public_address(3s)  -> #{public}"
  end
rescue StandardError => e
  puts "public_address failed: #{e.class}: #{e.message}"
end

rule 'Hostname resolution through TcpSocket'
%w[example.com].each do |host|
  socket = TcpSocket.new
  status = socket.connect(host, 80, SF::System::Time.seconds(3))
  line = "#{host}:80 -> #{status}"
  line += "  resolved to #{socket.remote_address}" if status == :done
  puts line
  socket.disconnect
rescue StandardError => e
  puts "#{host}:80 -> #{e.class}: #{e.message}"
end

puts "\nDone."

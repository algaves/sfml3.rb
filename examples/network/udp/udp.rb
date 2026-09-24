# frozen_string_literal: true

# UDP sockets are connectionless datagrams: each send is a self-contained
# message with its own destination, and each receive returns one whole message
# (or :not_ready). That is cheaper than TCP but lossy and unordered. This demo
# has a server echo every datagram back and a client send three and collect the
# replies. Console only; spawn the server, then the client:
#
#   bundle exec ruby -Ilib examples/network/udp/udp.rb          # server
#   bundle exec ruby -Ilib examples/network/udp/udp.rb client   # client
#
# Set PORT to use another port.

require 'sfml'
include SF::Network
include SF::System

PORT = Integer(ENV.fetch('PORT', 5001))

socket = UdpSocket.new

if ARGV[0] == 'client'
  abort 'bind failed' unless socket.bind(UdpSocket.any_port) == :done
  puts "client bound to port #{socket.local_port}"

  3.times do |i|
    status = socket.send("datagram #{i}", IpAddress::LOCAL_HOST, PORT)
    abort "send #{i} failed with #{status}" unless status == :done
  end

  3.times do
    data, address, port, status = socket.receive(1024)
    break unless status == :done

    puts "reply from #{address}:#{port} -> #{data.inspect}"
  end
else
  abort 'bind failed' unless socket.bind(PORT) == :done
  puts "udp server on port #{socket.local_port} (max datagram #{UdpSocket.max_datagram_size} bytes)"

  loop do
    data, address, port, status = socket.receive(1024)
    break unless status == :done

    puts "received #{data.inspect} from #{address}:#{port}"
    socket.send("ack: #{data}", address, port)
  end
end

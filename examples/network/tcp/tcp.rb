# frozen_string_literal: true

# TCP is a reliable, ordered byte stream, so SFML layers framing on top of it:
# `send_packet`/`receive_packet` carry a whole Packet and deliver it whole at
# the far end. The server accepts clients and echoes each packet back with a
# sequence number; the client sends three typed packets and prints the replies.
# Console-only, no display, no assets. Spawn the server, then the client:
#
#   bundle exec ruby -Ilib examples/network/tcp/tcp.rb          # server
#   bundle exec ruby -Ilib examples/network/tcp/tcp.rb client   # client
#
# Set PORT to use another port. Stop the server with Ctrl-C.

require 'sfml'
include SF::Network
include SF::System

PORT = Integer(ENV.fetch('PORT', 5000))

if ARGV[0] == 'client'
  socket = TcpSocket.new
  abort 'connect failed' unless socket.connect(IpAddress::LOCAL_HOST, PORT) == :done

  3.times do |i|
    outbound = Packet.new
    outbound.write_uint32(i)
    outbound.write_string("message #{i}")
    abort "send #{i} failed" unless socket.send_packet(outbound) == :done

    inbound = Packet.new
    break unless socket.receive_packet(inbound) == :done

    sequence = inbound.read_uint32
    puts "reply #{sequence}: #{inbound.read_string.inspect}"
  end

  socket.disconnect
else
  listener = TcpListener.new
  abort 'listen failed' unless listener.listen(PORT) == :done
  puts "tcp server on port #{listener.local_port} (Ctrl-C to stop)"

  loop do
    client, status = listener.accept
    next unless status == :done

    puts "client connected from #{client.remote_address}:#{client.remote_port}"
    loop do
      packet = Packet.new
      break unless client.receive_packet(packet) == :done

      sequence = packet.read_uint32
      message = packet.read_string
      puts "  [#{sequence}] #{message.inspect}"

      echo = Packet.new
      echo.write_uint32(sequence)
      echo.write_string("echo: #{message}")
      client.send_packet(echo)
    end

    puts 'client disconnected'
    client.disconnect
  end
end

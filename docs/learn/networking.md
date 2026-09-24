---
layout: default
title: Networking
parent: Learn
nav_order: 8
---

# Tutorial 8: Networking with Sockets & Packets

TCP for reliable streams, UDP for datagrams, `Packet` for typed serialization, `SocketSelector` for multiplexing. Statuses are symbols — always check them.

## TCP echo (reliable)

Server:

```ruby
require 'sfml'

listener = SF::Network::TcpListener.new
raise 'listen failed' unless listener.listen(5000) == :done
client, st = listener.accept
loop do
  data, st = client.receive(1024)
  break unless st == :done
  client.send(data)
end
```

Client:

```ruby
sock = SF::Network::TcpSocket.new
raise unless sock.connect(SF::Network::IpAddress::LOCAL_HOST, 5000) == :done
sock.send('hello')
data, st = sock.receive(1024)
puts data if st == :done
```

Non-blocking returns `:not_ready` / `:partial` — loop `send_partial` / `receive`.

## Typed packets

```ruby
out = SF::Network::Packet.new
out.write_int32(42)
out.write_string('hello')
sock.send_packet(out)

_in = SF::Network::Packet.new
sock.receive_packet(_in)
puts _in.read_int32.inspect
puts _in.read_string.inspect
```

Read/write order + types must mirror exactly; the cursor only moves forward (`clear` to reuse).

## UDP (unreliable datagrams)

```ruby
a = SF::Network::UdpSocket.new
a.bind(54000)
a.send('ping', SF::Network::IpAddress::LOCAL_HOST, 54001)
data, from_ip, from_port, st = a.receive(1024)
```

Unordered, possibly lost, truncated over `max_datagram_size` — use the sender tuple for replies.

## Multiplexing with SocketSelector

```ruby
sel = SF::Network::SocketSelector.new
sel.add(listener)
sel.add(udp_sock)
if sel.wait(SF::System::Time.seconds(1))
  puts 'listener ready' if sel.tcp_listener_ready?(listener)
  puts 'udp ready' if sel.udp_socket_ready?(udp_sock)
end
```

Re-`wait` then test `*_ready?` each loop; bare `wait` blocks forever.

## HTTP & FTP

```ruby
http = SF::Network::Http.new
http.set_host('example.com', 80)
req = SF::Network::HttpRequest.new
req.method = :get
req.uri = '/'
res = http.send_request(req)
puts res.status_name.inspect # :ok, :not_found, :connection_failed, …
```

HTTP only (no HTTPS yet). FTP: `connect` → `login`/`login_anonymous` → commands (`working_directory`, `directory_listing`, `download`/`upload` with `:binary` for non-text) → `disconnect`. Check `ok?` every step.

## Worked examples

- [Networking]({% link book/networking.md %}) — the Book's section: sockets, packets, TCP, UDP and file transfer.
- [TCP recipe]({% link book/net-tcp.md %}) — a console `TcpListener`/`TcpSocket` echo with typed `Packet`s.
- [API: Network]({% link api/network.md %}) — the full method tables.

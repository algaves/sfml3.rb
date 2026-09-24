---
layout: default
title: TCP
parent: Networking
grand_parent: "Part IV — Advanced"
nav_order: 3
---

# Recipe: TCP

TCP is a reliable, ordered byte stream: bytes arrive in the order sent, or the connection breaks. Because a stream has no message boundaries, SFML frames it for you — `send_packet`/`receive_packet` carry a whole `Packet` and deliver it whole at the far end. This recipe is a console client/server echo built that way.

> **Advanced**

## Goal

Two console processes. The server listens on a port, accepts clients and echoes each packet back with a sequence number; the client connects, sends three typed packets and prints the replies. Run the server in one terminal and the client in another.

## How the code works

### 1. Choose a side and connect

`PORT` is read from the environment (default `5000`), and `ARGV[0] == 'client'` decides the branch. The client creates a `TcpSocket` and calls `connect(IpAddress::LOCAL_HOST, PORT)`, aborting unless the status is `:done` so a missing server is obvious.

{% example ruby examples/network/tcp/tcp.rb 18-22 %}

### 2. Client: send three typed packets

The loop builds a `Packet` per iteration, writes a `uint32` sequence number and a string, then sends it whole with `send_packet`. `write_*` returning the packet and a failed send both abort, keeping the wire protocol strict.

{% example ruby examples/network/tcp/tcp.rb 24-28 %}

### 3. Client: read the replies and disconnect

A fresh `Packet` receives each reply; `break` leaves the loop when `receive_packet` is not `:done`. The fields are read back in the same order they were written and printed, then `socket.disconnect` closes the connection cleanly.

{% example ruby examples/network/tcp/tcp.rb 30-37 %}

### 4. Server: listen on the port

With no argument the script takes the server branch: a `TcpListener` is created and `listen(PORT)` binds it. `local_port` reports the port actually bound, which is printed so the client knows where to connect.

{% example ruby examples/network/tcp/tcp.rb 39-41 %}

### 5. Server: accept a client

`listener.accept` blocks until a client arrives and returns `[socket, status]`. The `next unless status == :done` guard skips a failed accept and loops again, then the client's remote address and port are printed.

{% example ruby examples/network/tcp/tcp.rb 43-47 %}

### 6. Server: read each packet

For each connected client, an inner `loop` makes a new `Packet` and calls `receive_packet`, which blocks for data. A non-`:done` status means the peer closed, so the loop `break`s; otherwise the sequence number and message are read in write order.

{% example ruby examples/network/tcp/tcp.rb 48-54 %}

### 7. Server: echo the reply

A second `Packet` echoes the data back — the same sequence number plus an `echo:` string — and `send_packet` returns it to the client. When the loops end the server prints that the client disconnected, calls `disconnect`, and returns to `accept` for the next one.

{% example ruby examples/network/tcp/tcp.rb 56-64 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `TcpListener` | Network | Waits for incoming connections | `new`, `listen`, `local_port`, `accept` |
| `TcpSocket` | Network | Connects and exchanges packets | `new`, `connect`, `send_packet`, `receive_packet`, `disconnect` |
| `Packet` | Network | Typed message serialisation | `new`, `write_uint32`, `write_string`, `read_uint32`, `read_string` |
| `IpAddress` | Network | Target address | `LOCAL_HOST`, `remote_address`, `remote_port` |
| `SocketStatus` | Network | Result of every call | `:done` |

See the [Network API]({% link api/network.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/network/tcp/tcp.rb` and run.

{% example ruby examples/network/tcp/tcp.rb %}

{: .note }
> For many simultaneous clients, do not spawn a thread per socket: `SocketSelector#wait` plus `tcp_socket_ready?` multiplexes them in one thread. Set `socket.blocking = false` when you would rather poll than block.

---
layout: default
title: UDP
parent: Networking
grand_parent: "Part IV — Advanced"
nav_order: 4
---

# Recipe: UDP

UDP is the opposite of TCP: connectionless datagrams, each a self-contained message with its own destination. It is cheaper and lower-latency but lossy and unordered, which makes it a good fit for fast-moving state where the next update supersedes a lost one. This recipe is a console echo.

> **Advanced**

## Goal

Two console processes. The server binds a port and echoes every datagram back to its sender; the client binds an ephemeral port, sends three datagrams and collects the replies. Run the server in one terminal and the client in another.

## How the code works

### 1. Choose a side and create the socket

`PORT` is read from the environment (default `5001`), and one `UdpSocket` is created up front for both branches. Because UDP is connectionless there is nothing to connect, so the same object only needs binding and an address per send.

{% example ruby examples/network/udp/udp.rb 18-20 %}

### 2. Client: bind an ephemeral port

The client branch first calls `socket.bind(UdpSocket.any_port)` to claim a free local port, since a socket must be bound before it can receive. `local_port` prints which port the OS handed out.

{% example ruby examples/network/udp/udp.rb 22-24 %}

### 3. Client: send three datagrams

`send(data, IpAddress::LOCAL_HOST, PORT)` — unlike TCP, every send names its destination explicitly. Each send returns a status, and a non-`:done` value aborts, so all three datagrams reach the server.

{% example ruby examples/network/udp/udp.rb 26-29 %}

### 4. Client: collect the replies

`socket.receive(1024)` returns `[data, address, port, status]`, one whole datagram per call. The client reads three of them, breaking if a receive is not `:done`, and prints the payload with the sender it came from.

{% example ruby examples/network/udp/udp.rb 31-36 %}

### 5. Server: bind the shared port

The server branch binds `PORT` so the client has a fixed destination to aim at, and prints the bound port along with `UdpSocket.max_datagram_size` — the largest payload a single datagram can carry.

{% example ruby examples/network/udp/udp.rb 38-39 %}

### 6. Server: receive and reply

The server loops on `receive`, and datagram boundaries mean one receive yields exactly one send. It prints the payload and its source, then replies with `send("ack: #{data}", address, port)`, reusing the address and port it just received.

{% example ruby examples/network/udp/udp.rb 41-47 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `UdpSocket` | Network | Datagram channel | `new`, `bind`, `send`, `receive`, `any_port`, `max_datagram_size` |
| `IpAddress` | Network | Destination / source address | `LOCAL_HOST` |
| `SocketStatus` | Network | Result of every call | `:done` |

See the [Network API]({% link api/network.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/network/udp/udp.rb` and run.

{% example ruby examples/network/udp/udp.rb %}

{: .note }
> UDP packets can be lost, duplicated or reordered; add a sequence number and drop stale datagrams if that matters. `send_packet`/`receive_packet` also exist on `UdpSocket` when you want `Packet` framing over datagrams.

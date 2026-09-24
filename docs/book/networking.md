---
layout: default
title: Networking
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 7
has_children: true
permalink: /book/networking
---

# Networking

`SF::Network` lets a Ruby process talk to another over the network. Everything is built on two ideas: an **address** (`IpAddress`) and a **socket** (a channel to send and receive bytes on). Above raw bytes sits **`Packet`**, a typed buffer that turns game state into a message the other side can decode without hand-rolled packing.

The surface is small:

- **Addresses** — `IpAddress.from_string`, `from_bytes`, `from_integer`, plus the constants `NONE`, `ANY`, `LOCAL_HOST` and `BROADCAST`. `local_address` and `public_address(timeout)` resolve through the OS.
- **Sockets** — `TcpSocket` and `TcpListener` for reliable, ordered streams; `UdpSocket` for connectionless datagrams. `SocketSelector` multiplexes many sockets in one thread.
- **Status** — every call returns a `SocketStatus` symbol (`:done` on success), so failures are checked rather than raised. Sockets can be `blocking = false` for non-blocking use.
- **Packets** — `Packet#write_*` and `Packet#read_*` for every primitive type, sent whole with `send_packet` / received with `receive_packet`.
- **Higher levels** — `Http` fetches a URL and `Ftp` lists and transfers files.

The recipes go from the smallest pieces up. Upstream reference: [SFML tutorials — Network](https://www.sfml-dev.org/tutorials/3.1/network/).

1. **[Sockets & Addresses]({% link book/net-sockets.md %})** — `IpAddress`, `SocketStatus`, and blocking vs non-blocking. Console.
2. **[Packets]({% link book/net-packets.md %})** — typed serialisation, in order. Console.
3. **[TCP]({% link book/net-tcp.md %})** — a reliable client/server echo with packets. Console.
4. **[UDP]({% link book/net-udp.md %})** — connectionless datagrams. Console.
5. **[File Transfer]({% link book/net-file-transfer.md %})** — `Http` and `Ftp` above the socket layer. Console.

{: .note }
> All five run in a terminal and need no window. The transport recipes start a server in one terminal and a client in another; set `PORT` to avoid clashes. `Sftp` (SFML 3.1's FTP replacement) is not part of the bound CSFML surface.

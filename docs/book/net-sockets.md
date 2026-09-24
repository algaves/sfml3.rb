---
layout: default
title: Sockets & Addresses
parent: Networking
grand_parent: "Part IV — Advanced"
nav_order: 1
---

# Recipe: Sockets & Addresses

Before anything can be sent, you need an address to send it to and a socket to send it on. An `IpAddress` is a 32-bit address you can build from a string, four bytes or an integer; a socket is a channel whose calls each return a `SocketStatus`. This console recipe covers the address surface and the status/blocking conventions every later recipe relies on.

> **Advanced**

## Goal

A console script (no window) that builds and prints addresses every way, shows the named constants, resolves the local and public addresses best-effort, and demonstrates `SocketStatus` and non-blocking mode.

## How the code works

### 1. Build one address three ways

`IpAddress.from_string('127.0.0.1')`, `IpAddress.from_bytes(127, 0, 0, 1)` and `IpAddress.from_integer(2_130_706_433)` each describe the same loopback address. The script prints all three, then checks with `==` that they compare equal and uses `to_integer` to round-trip back to the number.

{% example ruby examples/network/sockets/sockets.rb 20-28 %}

### 2. Print the named constants

`NONE`, `ANY`, `LOCAL_HOST` and `BROADCAST` are ready-made addresses rather than ones you construct. Iterating the four into `to_s` and `to_integer` shows the numeric value behind each name.

{% example ruby examples/network/sockets/sockets.rb 30-34 %}

### 3. Resolve the local address

`IpAddress.local_address` asks the operating system which address this machine uses. A `Clock` records how long the lookup took and the whole call is wrapped in `rescue` so a resolver failure is reported, not raised.

{% example ruby examples/network/sockets/sockets.rb 36-43 %}

### 4. Resolve the public address, with a timeout

`IpAddress.public_address(Time.seconds(2.0))` reaches out to the internet and gives up after two seconds. That call is best-effort, so the example again rescues and prints the failure instead of hanging the script.

{% example ruby examples/network/sockets/sockets.rb 45-52 %}

### 5. Read the SocketStatus

Every network call returns a symbol rather than raising. Connecting to a port nothing listens on comes back as `:error` quickly on loopback, and the script prints `status.inspect` so the value is visible; a rescue covers the odd case that does raise.

{% example ruby examples/network/sockets/sockets.rb 54-63 %}

### 6. Choose blocking behaviour

A fresh `TcpSocket` is blocking by default, which `blocking?` confirms. Setting `socket.blocking = false` makes every later call return at once with `:not_ready` instead of waiting — the convention a game loop uses to stay responsive.

{% example ruby examples/network/sockets/sockets.rb 65-70 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `IpAddress` | Network | A 32-bit address | `from_string`, `from_bytes`, `from_integer`, `to_integer`, `local_address`, `public_address` |
| `IpAddress` constants | Network | Named addresses | `NONE`, `ANY`, `LOCAL_HOST`, `BROADCAST` |
| `TcpSocket` | Network | A socket to show status/blocking | `connect`, `disconnect`, `blocking=`, `blocking?` |
| `SocketStatus` | Network | Result of every call | `:done`, `:error`, `:not_ready` |
| `Clock` | System | Timing the resolvers | `new`, `elapsed_time` |

See the [Network API]({% link api/network.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/network/sockets/sockets.rb` and run.

{% example ruby examples/network/sockets/sockets.rb %}

{: .note }
> `TcpSocket`, `TcpListener`, `UdpSocket` and `SocketSelector` all share these status and blocking conventions. `public_address` needs the internet, so treat a failure there as normal, not a bug.

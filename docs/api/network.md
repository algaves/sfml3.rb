---
layout: default
title: Network
parent: API Reference
nav_order: 5
---

# Network API

Sockets, HTTP, FTP, IP addresses, packets, and listeners.

## Key classes

| Class | Purpose |
|---|---|
| `SF::Network::Ftp` | FTP client |
| `SF::Network::Http` | HTTP client |
| `SF::Network::TcpSocket` | TCP socket |
| `SF::Network::UdpSocket` | UDP socket |
| `SF::Network::TcpListener` | TCP connection listener |
| `SF::Network::IpAddress` | IP address representation |
| `SF::Network::Packet` | Binary data packet |
| `SF::Network::SocketSelector` | Socket I/O multiplexing |

See `sig/network/` and `ext/network/` for full method signatures and documentation.

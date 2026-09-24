---
layout: default
title: Packets
parent: Networking
grand_parent: "Part IV — Advanced"
nav_order: 2
---

# Recipe: Packets

Raw bytes are hard to get right: you have to agree on sizes, order and endianness, then pack and unpack by hand. `Packet` does that for you. You write values in order, send the whole buffer, and the other side reads them back in the same order and type — the safest way to move game state.

> **Advanced**

## Goal

A console script (no window) that writes every supported type into a `Packet`, reads them back in order, and demonstrates a small tagged message protocol.

## How the code works

### 1. Write the integer types in order

A new `Packet` starts empty, and each `write_*` call appends a value and advances the cursor. The script writes `write_bool`, then the signed and unsigned widths from `write_int8` up to `write_uint64`, so the order of the calls is the order on the wire.

{% example ruby examples/network/packets/packets.rb 19-29 %}

### 2. Add the fractional and string fields

`write_float`, `write_double` and `write_string` finish the buffer with the remaining types. `data_size` then reports the total byte count and `read_position`/`end_of_packet?` show that no read has happened yet.

{% example ruby examples/network/packets/packets.rb 30-34 %}

### 3. Read them back in the same order

A second `Packet` is filled with `append(out.data)` and read with the matching `read_*` calls. Order and type mirror the writes exactly — that is the one rule to remember, and the output doubles each value back.

{% example ruby examples/network/packets/packets.rb 36-47 %}

### 4. Finish the reads and check the end

The last reads pull out the `float`, `double` and `string` (the string is `inspect`ed to show its quotes). `end_of_packet?` then returns true, proving every byte the writer produced has been consumed.

{% example ruby examples/network/packets/packets.rb 48-51 %}

### 5. Encode a tagged message

`player_update` shows the protocol pattern: prefix the message with a type tag (`write_uint8(1)`) and then its fields, returning the finished packet. A tag plus a fixed field layout is what lets a reader know what follows.

{% example ruby examples/network/packets/packets.rb 53-64 %}

### 6. Decode by dispatching on the tag

`read_message` reads the tag first and a `case` on it chooses the matching reader, pulling the id, coordinates and health back in order. The script builds one update, prints its size and the decoded hash, then falls through to `:unknown` for any other tag.

{% example ruby examples/network/packets/packets.rb 66-77 %}

### 7. Remember the rules of thumb

The closing lines spell out the contract: reads must mirror writes in order and type, `write_*` returns the packet so calls chain, and `append`/`clear!`/`copy` reuse buffers instead of allocating one per message.

{% example ruby examples/network/packets/packets.rb 79-81 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Packet` | Network | Typed byte buffer | `write_*`, `read_*`, `append`, `clear!`, `copy` |
| `Packet` inspection | Network | Size and cursor | `data_size`, `read_position`, `end_of_packet?` |

See the [Network API]({% link api/network.md %}) for the full list of readers and writers. The [TCP]({% link book/net-tcp.md %}) and [UDP]({% link book/net-udp.md %}) recipes send these packets over the wire.

## The complete script

The complete program, ready to copy into `examples/network/packets/packets.rb` and run.

{% example ruby examples/network/packets/packets.rb %}

{: .note }
> Reads must mirror the writes exactly, in order and in type. `write_*` returns the packet, so a message can be built by chaining calls.

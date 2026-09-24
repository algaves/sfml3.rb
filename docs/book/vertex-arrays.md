---
layout: default
title: Vertex Arrays
parent: "Shaders & Vertex Arrays"
grand_parent: "Part IV — Advanced"
nav_order: 2
---

# Recipe: Vertex Arrays

A `VertexArray` is an ordered list of vertices plus a **primitive** that says how to connect them. It is the lowest-level drawing tool SFML exposes and the substrate under the shapes, sprites and text — worth knowing because it turns thousands of objects into a single draw call.

> **Advanced**

## Goal

A 900×560 window showing one ring of twelve coloured vertices, re-read as each of the six primitive types, and a batched mode that draws 1500 triangles from one array. Press `1`–`6` to pick a primitive, `b` to toggle batching; escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring the subsystem names into scope. One `Font` is loaded from the assets directory, and the `text` helper wraps `Text.new` with a fixed colour and position so every readout is one call.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 15-29 %}

### 2. The primitive list, sizes and the window

`PRIMITIVES` lists the six primitive symbols in the order the number keys select them. The window's size, ring centre and vertex count are fixed next, and `frame_rate = 60` caps the loop.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 31-40 %}

### 3. Build the ring of twelve vertices

`COUNT.times` walks a full circle: `angle` spaces each vertex evenly and `Math.cos`/`Math.sin` turn it into an x/y position. Each `Vertex.new([x, y], color)` also stores a colour whose hue sweeps with the index, so the connection order is visible.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 42-50 %}

### 4. Build the batched array of 1500 triangles

A second `VertexArray` with `primitive = :triangles` holds 1500 small triangles built once under `srand(11)`. Three `Vertex` objects per triangle mean thousands of vertices live in one array and can be drawn in a single call.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 52-65 %}

### 5. Keep state and read the keys

`index = 3` and `batched = false` hold the current mode. In the event loop `:b` toggles batching while `:num1`..`:num6` set `index` into `PRIMITIVES`, and `:escape` quits.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 67-87 %}

### 6. Draw the batch when batching is on

When `batched` is true the whole array goes out with one `window.draw(batch)`. The HUD reports `batch.vertex_count`, showing how many vertices that single draw call covered.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 89-97 %}

### 7. Draw the ring with the chosen primitive

Otherwise `ring.primitive = PRIMITIVES[index]` reinterprets the same twelve vertices, and `window.draw(ring)` draws them. The HUD reads `ring.vertex_count` and `ring.vertex(...)` back to show the array's state, then `display!` presents the frame.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb 98-108 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `VertexArray` | Graphics | CPU-side vertex list | `new`, `primitive=`, `append`, `vertex_count`, `vertex`, `clear!` |
| `Vertex` | Graphics | Position, colour, texture coords | `new`, `position`, `color` |
| `VertexBuffer` | Graphics | GPU-resident vertices | `available?`, `new`, `update`, `usage=` |
| `Window` | Window | Render surface | `new`, `draw`, `poll_events!` |
| `Keyboard` / `Event` | Window | Primitive and batch toggles | `key_pressed?`, `poll_events!` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/vertex_arrays/vertex_arrays.rb` and run.

{% example ruby examples/graphics/vertex_arrays/vertex_arrays.rb %}

{: .note }
> The Tilemaps recipe draws a whole level as one `VertexArray` of textured triangles, and the Particle System rebuilds one every frame. Both are this class at work; [Shaders]({% link book/shader-wave.md %}) shows the program that can shade it.

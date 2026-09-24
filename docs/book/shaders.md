---
layout: default
title: "Shaders & Vertex Arrays"
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 2
has_children: true
permalink: /book/shaders
---

# Shaders & Vertex Arrays

Two ways to take control of what the GPU draws. A **Shader** is a small GLSL program that runs per vertex or per pixel; a **VertexArray** (or its GPU-resident cousin, `VertexBuffer`) is the raw geometry you feed it. They are often used together — custom geometry shaded by a custom program — so they share a section.

## Shaders

- `Shader.available?` reports whether shaders work on this driver; always guard on it.
- `Shader.from_memory(vertex_src, fragment_src)` compiles and links a program.
- Uniforms carry per-frame data to the GPU: `set_float`, `set_vec2`, and the other typed setters.
- A shader travels with one draw inside a `RenderState`, not globally: `state.shader = shader`, then `window.draw(target, state)`.

## Vertex arrays

- `VertexArray.new` plus `primitive=` builds geometry CPU-side; `:points`, `:lines`, `:line_strip`, `:triangles`, `:triangle_strip` and `:triangle_fan` say how vertices connect.
- Every `Vertex` carries a position, a colour and texture coordinates, so a batched array can draw thousands of triangles in one call.
- `VertexBuffer` uploads the same vertices to the GPU once and lets you `update` regions — the right tool for geometry that changes every frame.

## The recipes

1. **[Shader Wave Effect]({% link book/shader-wave.md %})** — a full-screen GLSL palette with `u_time`, `u_resolution` and `u_mouse` uniforms.
2. **[Vertex Arrays]({% link book/vertex-arrays.md %})** — every primitive type, per-vertex colour, and batching thousands of triangles into one draw.

{: .note }
> Shader support depends on the driver; `Shader.available?` and `Shader.geometry_available?` let an example fall back to a plain message instead of crashing. `VertexBuffer.available?` guards the GPU-resident path.

---
layout: default
title: Particle Systems
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 3
---

# Recipe: Particle Systems

A particle system is a pool of many tiny, short-lived pieces of geometry. Rather than one object per spark, the loop keeps a plain array of particles, spawns them at a rate, integrates their motion for a fraction of a second, fades them out, and recycles the slots. All of them are drawn from a single `VertexArray`, so the frame cost is the vertex count, not the object count.

> **Intermediate**

## Goal

A 900×560 window with a fountain of ~600 particles per second rising from an emitter, pulled down by gravity and fading from yellow to red as they die. Clicking bursts 250 particles at the pointer; space toggles gravity, `+`/`-` change the spawn rate, `c` clears. Escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring the subsystem names into scope. One `Font` is loaded from the assets directory, and the `text` helper wraps `Text.new` with a fixed colour and position so the status readout is one call.

{% example ruby examples/graphics/particles/particles.rb 15-29 %}

### 2. The particle struct, sizes and system state

`PARTICLE` is a `Struct` of just the hot fields, so one particle is plain data and thousands are cheap to allocate; `MAX_PARTICLES` caps the pool. The window and the system's state — clock, particle list, spawn rate and accumulator, gravity and emitter — are created next.

{% example ruby examples/graphics/particles/particles.rb 31-47 %}

### 3. The spawn helper

`spawn` emits `count` particles from `(x, y)`, each with a random angle, a speed scaled between 0.2 and 1.0 of `speed`, and a random lifetime stored as a new `PARTICLE`. Both the steady stream and the click burst go through it.

{% example ruby examples/graphics/particles/particles.rb 49-57 %}

### 4. Poll events: bursts, gravity, rate and clear

A `'mouse-button-pressed'` reads `Mouse.position(window)` and spawns a 250-particle burst. Key presses toggle gravity on `:Space`, step the spawn rate on `:Equal`/`:Hyphen`, clear the pool on `:c`, and quit on `:escape`.

{% example ruby examples/graphics/particles/particles.rb 59-76 %}

### 5. Delta time and rate-based spawning

`clock.restart!.as_seconds` gives `dt`, clamped to 0.05 after a stall so nothing teleports. `spawn_accumulator` gains `spawn_rate * dt` and one particle is emitted per whole unit, which makes the visual density independent of the frame rate.

{% example ruby examples/graphics/particles/particles.rb 78-86 %}

### 6. Integrate and retire

`particles.reject!` integrates each particle as it visits it: gravity adds to velocity, velocity to position, `dt` is taken off the lifetime, and any particle at or below zero is dropped. If the pool is still over `MAX_PARTICLES`, the oldest are shifted off.

{% example ruby examples/graphics/particles/particles.rb 88-96 %}

### 7. Build one quad per particle, coloured by age

A fresh `VertexArray` of `:triangles` is filled each frame. `fade` is the fraction of life left and drives the green, blue and alpha channels so particles cool from yellow to red, and six `Vertex` objects form the two triangles of each quad.

{% example ruby examples/graphics/particles/particles.rb 98-119 %}

### 8. Draw the batch and the HUD

`window.draw(mesh)` sends every particle in one call, which is why the cost is the vertex count rather than the object count. The `text` helper prints the live particle count, spawn rate and gravity state, and `display!` presents the frame.

{% example ruby examples/graphics/particles/particles.rb 121-130 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `VertexArray` / `Vertex` | Graphics | Every particle in one batch | `new`, `primitive=`, `append` |
| `Clock` | System | Delta time | `restart!`, `elapsed_time` |
| `Mouse` | Window | Burst origin | `position` |
| `Keyboard` / `Event` | Window | Rate, gravity, clear, quit | `key_pressed?`, `poll_events!` |
| `Struct` (Ruby) | — | One particle's fields | `new` |

See the [Graphics API]({% link api/graphics.md %}) for `VertexArray`, and [Custom Shape]({% link book/custom-shape.md %}) for defining geometry as a `Shape` subclass instead.

## The complete script

The complete program, ready to copy into `examples/graphics/particles/particles.rb` and run.

{% example ruby examples/graphics/particles/particles.rb %}

{: .note }
> The pattern scales: add fields for rotation, size-over-life or a texture, and swap the `:triangles` batch for `VertexBuffer` when the geometry stops changing. The cap on the pool is what keeps a burst from freezing the frame.

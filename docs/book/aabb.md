---
layout: default
title: AABB Collisions
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 4
---

# Recipe: AABB Collisions

Axis-aligned bounding boxes (AABBs) are the workhorse of 2D collision: a rectangle that contains an object, tested against other rectangles. This recipe builds a tiny platformer on top of them — gravity, jumping and solid platforms — with the overlap drawn on screen.

> **Intermediate**

## Goal

A 900×560 window with a blue player box that falls, lands, runs and jumps across four static platforms. Every solid box is an AABB, and when the player overlaps one the intersection rectangle flashes red so the resolution is visible. Arrow keys move, space jumps, `R` resets, escape quits.

## How the code works

### 1. Tuning constants

Gravity, jump impulse, acceleration, top speed, friction and the player size are named constants at the top. Keeping them together makes the feel of the platformer easy to adjust without hunting through the physics.

{% example ruby examples/graphics/aabb/aabb.rb 29-36 %}

### 2. The player and its platforms

The player is a `RectangleShape` whose origin is its centre, and `SOLIDS` lists four boxes in `[x, y, w, h]` form. Each is turned into a styled `RectangleShape`, and the same array is used both for drawing and for collision.

{% example ruby examples/graphics/aabb/aabb.rb 41-50 %}

### 3. Resolve one axis at a time

`move_axis` moves the player on one axis, wraps it in a `Rect` and tests that box against each solid's `global_bounds` with `intersects?`. On a hit it takes `intersection` for the overlap and snaps the player to the solid's edge, zeroing that axis's velocity; resolving the axes separately stops corners from sticking.

{% example ruby examples/graphics/aabb/aabb.rb 72-100 %}

### 4. Key events: jump, reset, quit

Escape closes the window, Space or Up sets `vy` to the jump impulse but only when `on_ground`, and `R` respawns the player and clears the resolution counter.

{% example ruby examples/graphics/aabb/aabb.rb 108-114 %}

### 5. Gravity and horizontal control

Held arrow keys accelerate `vx`, which is clamped to the speed cap and multiplied by friction when neither key is held. Gravity is added to `vy` every frame and clamped to a terminal speed.

{% example ruby examples/graphics/aabb/aabb.rb 118-124 %}

### 6. Move, collide and land

The player is moved horizontally and then vertically through `move_axis`, so a wall stops horizontal motion but leaves the vertical one untouched. The newest overlap and the landing flag are kept for drawing and jumping, and `player.position = [x, y]` applies the result.

{% example ruby examples/graphics/aabb/aabb.rb 126-137 %}

### 7. Draw the boxes and the overlap

Solids are drawn first, then a transparent white outline marks the player's `global_bounds`, followed by the player itself. When an overlap was found, the intersection rectangle is filled translucent red — the visual proof the maths is working.

{% example ruby examples/graphics/aabb/aabb.rb 139-157 %}

### 8. The HUD and present

A text line reports velocity, `on_ground`, the resolution count and the latest overlap size. `display!` then presents the frame and the loop re-checks `window.open?`.

{% example ruby examples/graphics/aabb/aabb.rb 159-166 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Rect` | Graphics | The AABB: test and overlap | `new`, `intersects?`, `intersection`, `left`, `top`, `width`, `height` |
| `Shape#global_bounds` | Graphics | A shape's AABB | `global_bounds` |
| `RectangleShape` | Graphics | Player and platforms | `new`, `position`, `position=`, `origin=`, `fill_color=` |
| `Keyboard` / `Event` | Window | Move and jump input | `key_pressed?`, `poll_events!` |
| `Window` | Window | Surface | `new`, `clear!`, `draw`, `display!` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/aabb/aabb.rb` and run.

{% example ruby examples/graphics/aabb/aabb.rb %}

{: .note }
> `Rect#contains?` is the point-in-box test (used by [Triggers, Events & ECS]({% link book/ecs.md %})); `intersects?`/`intersection` are the box-vs-box pair. Keep AABBs axis-aligned — once a shape rotates, its `global_bounds` grows to fit and is no longer a tight box.

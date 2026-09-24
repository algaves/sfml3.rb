---
layout: default
title: Triggers, Events & ECS
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 5
---

# Recipe: Triggers, Events & ECS

This recipe is about architecture: an **entity-component-system** (ECS) that keeps data in component tables and logic in systems, plus **trigger areas** that push events onto a queue other systems consume. It is a small, dependency-free pattern that scales from a demo to a real game.

> **Advanced**

## Goal

An 860×560 world where the player (arrow keys) walks into glowing trigger zones, which queue an `:entered` event that a score system consumes. Drones patrol under a movement system, and space/`D` add and remove entities on the fly. Escape quits.

## How the code works

### 1. Entities are ids, components are tables

`World#initialize` allocates nothing but hashes — `@positions`, `@velocities`, `@shapes`, `@zones`, `@tags` and `@inside` — all keyed by an integer id. An entity is simply the set of rows that share its id, and `@events` is the queue other systems read.

{% example ruby examples/subsystems/ecs/ecs.rb 36-45 %}

### 2. Spawn and despawn manage ids

`spawn` hands out the next id and writes only the components it was given; `despawn` deletes that id from every table at once, so an entity leaves cleanly with no per-type bookkeeping.

{% example ruby examples/subsystems/ecs/ecs.rb 47-60 %}

### 3. movement_system integrates and bounces

The system iterates just the velocity table, adds each velocity to its position and flips the sign (clamping) at the window edges. Entities without a velocity are never visited, which is the point of a data-oriented system.

{% example ruby examples/subsystems/ecs/ecs.rb 68-83 %}

### 4. trigger_system fires on the crossing frame

For each zone `Rect`, `contains?` tests the player; an event is pushed only when the player is inside now but was outside last frame, using the `@inside` table as the edge memory. `consume_events` then hands the batch to whoever asks and clears the queue.

{% example ruby examples/subsystems/ecs/ecs.rb 86-99 %}

### 5. render_system copies positions and draws

This system walks the shape table, copies the matching position component onto the shape and calls `target.draw`, so geometry and data stay in sync without either knowing about the other. The window, spawn helpers and entities are built up from plain Ruby afterwards.

{% example ruby examples/subsystems/ecs/ecs.rb 101-107 %}

### 6. Populate the world

`World.new` is filled with a player, two velocity-driven drones as `:drone` entities, and two glowing trigger zones that carry a `Rect` component plus a translucent shape. `spawn` returns the ids the loop keeps for the player.

{% example ruby examples/subsystems/ecs/ecs.rb 126-148 %}

### 7. Poll events to spawn and remove

`window.poll_events!` handles the close request, escape, space to spawn a drone with a random velocity, and `D` to despawn the newest — entity lifecycle driven straight from input.

{% example ruby examples/subsystems/ecs/ecs.rb 150-166 %}

### 8. Run the systems, drain events and draw

The loop writes the player's position component from the keyboard, runs `movement_system` and `trigger_system`, drains the queue to bump the score, then clears, renders and draws the HUD with `window.display!`.

{% example ruby examples/subsystems/ecs/ecs.rb 168-194 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `World` (this recipe) | — | Component tables and systems | `spawn`, `despawn`, `ids`, `movement_system`, `trigger_system`, `consume_events`, `render_system` |
| `Rect` | Graphics | Trigger areas; point-in-box test | `new`, `contains?`, `left`, `top`, `width`, `height` |
| `CircleShape` / `RectangleShape` | Graphics | Entity sprites | `new`, `origin=`, `position=`, `fill_color=` |
| `Hash` / `Array` (Ruby) | — | Components and systems | `[]`, `delete`, `each` |
| `Keyboard` / `Event` | Window | Input system and spawn/remove keys | `key_pressed?`, `poll_events!` |

See the [Graphics API]({% link api/graphics.md %}) for `Rect` and the shapes; the ECS itself is plain Ruby.

## The complete script

The complete program, ready to copy into `examples/subsystems/ecs/ecs.rb` and run.

{% example ruby examples/subsystems/ecs/ecs.rb %}

{: .note }
> The same shape works with `Rect#intersects?` for solid collisions ([AABB Collisions]({% link book/aabb.md %})) and `contains?` for areas that only fire events, as here. Because systems read only the tables they need, new behaviour is a new method over existing data.

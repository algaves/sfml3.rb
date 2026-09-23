---
layout: default
title: System
parent: API Reference
nav_order: 4
---

# System API

Time, clocks, threading primitives, vectors, rects, and utilities.

## Key classes/modules

| Class/Module | Purpose |
|---|---|
| `SF::System::Clock` | High-resolution timer |
| `SF::System::Time` | Time duration representation |
| `SF::System::Sleep` / `SF.sleep!` | Thread sleep |
| `SF::System::Vector2` / `SF::System::Vector2f` / `SF::System::Vector2i` | 2D vectors |
| `SF::System::Vector3` / `SF::System::Vector3f` / `SF::System::Vector3i` | 3D vectors |
| `SF::System::Rect` / `SF::Graphics::Rect` / `SF::Graphics::FloatRect` | Axis-aligned rectangles |
| `SF::System::InputStream` | Stream abstraction |

See `sig/system/` and `ext/system/` for details.

---
layout: default
title: Book
nav_order: 5
has_children: true
permalink: /book
---

# The Book

A cookbook for `sfml3-rb`. Recipes are ordered by difficulty — **Essential → Basic → Intermediate → Advanced** — not by module, so you can start at the top and work down. Every recipe is self-contained: it explains the goal, the method and the classes involved, then ends with the complete, runnable script inlined from the real file under `examples/`.

> Looking for raw scripts to copy without the walkthrough? See the separate [Examples]({% link examples/index.md %}) section.

## Recipe format

Each recipe follows the same shape so you always know where to look:

- **Goal** — what you are about to build.
- **How the code works** — the script broken into numbered parts, each explained and
  showing the exact lines it talks about.
- **Ingredients** — a table of every class the recipe touches and where it lives.
- **The complete script** — the whole file at the bottom, ready to copy, paste and run.

Recipes need a real display and run until their window is closed; headless machines can run them under `xvfb-run -a`.

## Parts

1. **[Part I — Essential]({% link book/part-1-essentials.md %})** — the window, events, and the Transformable/Shape/Drawable core.
2. **[Part II — Basic]({% link book/part-2-basic.md %})** — input devices, the asset pipeline and animation.
3. **[Part III — Intermediate]({% link book/part-3-intermediate.md %})** — collision, tilemaps, audio analysis and custom geometry.
4. **[Part IV — Advanced]({% link book/part-4-advanced.md %})** — cameras, shaders, particles, event-driven audio, ECS, GUI and networking, several as multi-page sections.

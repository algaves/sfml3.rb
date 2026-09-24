---
layout: default
title: Views
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 1
has_children: true
permalink: /book/views
---

# Views

A `View` is SFML's camera: a rectangle over the world that decides both *which part* of the scene is drawn and *where* on the target it lands. Everything in this section is the same class used four ways — following a player, filling a viewport, moving at a different speed, or scrolling under your control.

The tools are small:

- **The view rectangle** — `View.from_rect(Rect.new(0, 0, w, h))` creates one the size of the window; `view.center = [x, y]` points it at a spot in the world; `view.size` reads its extent.
- **Zoom** — `view.zoom(factor)` scales the rectangle in place (a factor above `1` shows more world, below `1` shows less).
- **Viewports** — `view.viewport = [x, y, w, h]` is a normalised (0–1) rectangle on the target, so several views can share one window.
- **Scissor** — `view.scissor = rect` clips that view's output to a target rectangle when a viewport alone is not enough.
- **Applying it** — `window.view = view` makes the next draws use it; `window.view = window.default_view` restores pixel coordinates for a HUD.

## The four views

1. **[Camera]({% link book/views-camera.md %})** — follow a player through a large world, clamped so the edge never shows the void.
2. **[Split-Screen]({% link book/views-split.md %})** — two viewports, two independent cameras in one window.
3. **[Parallax]({% link book/views-parallax.md %})** — a second camera moving at half speed for depth.
4. **[Scrolling]({% link book/views-scrolling.md %})** — pan the view yourself across a world larger than the window.

{: .note }
> A view is per-draw state, not global: set `window.view` before the draws that should use it, and restore `window.default_view` before the HUD. `window.map_pixel_to_coords` converts a window point back into world coordinates under the active view.

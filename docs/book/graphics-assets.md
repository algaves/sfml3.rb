---
layout: default
title: Graphics Assets
parent: "Part II — Basic"
grand_parent: Book
nav_order: 5
---

# Recipe: Graphics Assets

The whole sprite/texture/image path in one window: it loads a sprite sheet and animates a cell through `Sprite#texture_rect`, builds a `Texture` at runtime from `Image.from_pixels`, edits an `Image` pixel by pixel, and renders a live scene into a `RenderTexture` that is drawn back as a sprite. `S` toggles smoothing, `P` toggles repeating.

> **Basic**

## Goal

A 900×560 window with an animated bird loaded from a vendored sprite sheet, a rotating gem, a checkerboard backdrop generated at runtime, an edited `Image` reported in the HUD, and an off-screen `RenderTexture` (five spinning bars) shown as a sprite. `S`/`P` toggle the checker's filtering, escape quits.

## How the code works

### 1. Load the font and build labels

`ASSETS` locates the example's own folder and `FONT` loads the bundled font once. The `text` helper wraps `Text.new` with a colour and position so captions can be created inline.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 21-28 %}

### 2. The sprite sheet and the cell map

`Texture.from_file` loads the vendored `sprites.png` once into a GPU texture, and `SPRITES` names its 16×16 cells as `[column, row]`. The `sprite` helper looks a name up, builds a `Sprite`, selects that cell with `texture_rect` and applies an optional scale.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 30-43 %}

### 3. Open the window, then the bird and gem

The window is created and capped at 60 fps, then `BIRD_FRAMES` defines the four-step flap order and `sprite` builds the scaled bird and the gem. The gem gets an `origin` at its centre so it can spin about the middle rather than the corner.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 69-82 %}

### 4. A runtime checker and an edited Image

`procedural_texture` builds a 32×32 checker by filling a binary pixel `String` and calling `Image.from_pixels` then `Texture.from_image`; `repeated = true` lets it tile, and a `RectangleShape` backdrop carries it via `texture`/`texture_rect`. A separate 16×16 gradient shows `set_pixel`, `pixel`, `save_to_memory` and `flip_vertically!` on a plain `Image`.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 84-99 %}

### 5. Render a scene off-screen

`RenderTexture.new([240, 150])` is a draw target that is not the window. `render_offscreen` clears it, marks it `active`, draws five rotating bars, calls `display` and releases it, and the result is wrapped in a `Sprite` built from `render_texture.texture` just like any loaded texture.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 102-124 %}

### 6. Handle key events

`'closed'` and `:escape` close the window. `:s` flips `checker.smooth?` (blurry versus crisp when scaled) and `:p` flips `checker.repeated?`, changing how the backdrop samples the checker.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 129-143 %}

### 7. Advance the animation

Every 0.14 s on the `Clock` the frame index steps and wraps through `BIRD_FRAMES`; the selected cell is written to `bird.texture_rect`, and the gem's rotation advances. There is no animation class — the cell index is the animation, and `texture_rect` is the only state.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 145-153 %}

### 8. Draw the scene and HUD

The off-screen render is refreshed, then the backdrop, bird, gem and off-screen sprite are drawn in order. The HUD names the sheet size and image results and reports `smooth?`/`repeated?` before `display!` presents the frame.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb 155-173 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Window` | Window | Render surface (created before textures) | `new`, `frame_rate=`, `draw`, `display!`, `size` |
| `Texture` | Graphics | GPU image, includes `RenderTexture#texture` | `from_file`, `from_image`, `smooth=`, `smooth?`, `repeated=`, `repeated?` |
| `Image` | Graphics | CPU pixel buffer | `from_pixels`, `set_pixel`, `pixel`, `save_to_memory`, `flip_vertically!`, `size` |
| `Sprite` | Graphics | Drawable textured quad | `new`, `texture_rect=`, `position=`, `scale=`, `origin=`, `rotation` |
| `RenderTexture` | Graphics | Off-screen render target | `new`, `active=`, `clear`, `draw`, `display`, `texture`, `smooth=` |
| `RectangleShape` | Graphics | Backdrop quad that carries the checker | `new`, `position=`, `texture=`, `texture_rect=` |
| `Clock` | System | Frame/anim timing | `new`, `elapsed_time`, `restart!` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/graphics_assets/graphics_assets.rb` and run.

{% example ruby examples/subsystems/graphics_assets/graphics_assets.rb %}

{: .note }
> Create the window before any `Texture` — textures need a GL context — and keep the `Texture` object alive as long as a `Sprite` uses it. See [Sprite Animation]({% link book/sprite-animation.md %}) and [Tileset]({% link book/tileset.md %}) for focused takes on the same APIs.

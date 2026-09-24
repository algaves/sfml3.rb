---
layout: default
title: Character Animation
parent: "Part II — Basic"
grand_parent: Book
nav_order: 6
---

# Recipe: Character Animation

Sprite animation with named clips over a runtime-built atlas. The atlas is drawn into an `Image` pixel by pixel (three rows: idle, walk, run — four frames each), uploaded once with `Texture.from_image`, and animated by advancing `Sprite#texture_rect` on a `Clock`; `1`/`2`/`3` switch clips, space pauses, `+`/`-` change speed and `F` flips.

> **Basic**

## Goal

An 800×600 window with an 8×-scaled character that idles, walks or runs on the spot, driven by a 4×3 atlas generated in code. `1`/`2`/`3` pick the clip, space pauses, `+`/`-` change speed, `F` faces left, escape quits.

## How the code works

### 1. Set up helpers, atlas geometry and palette

The font and `text` helper provide HUD labels, while `CELL`, `COLUMNS` and `ROWS` describe the atlas as a 96×72 image with one row per clip and one column per frame. `SKIN`, `SHIRTS` and `PANTS` fix the colours the pixel-drawing loop will use.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 21-35 %}

### 2. Draw one frame: the layer helpers and pose

`draw_character` works entirely through `image.set_pixel`: `put` bounds-checks a single pixel and `fill` paints a rectangle through it. `swing` comes from the column and doubles on the run row, `bob` lifts the body on one idle frame, and `shirt` selects the row's colour — the pose is all derived from `row` and `column`.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 39-49 %}

### 3. Draw one frame: the body parts

With the pose fixed, the same function stamps the head, torso, two swinging arms, two legs and two eyes in order. Each call is just an offset rectangle in the 24×24 cell, so the whole character is described by six `fill`s and two eyes.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 51-59 %}

### 4. Assemble the atlas and describe the clips

`build_atlas` starts from a fully transparent `Image.from_color` and calls `draw_character` for all twelve cells, offsetting each by `column * CELL` and `row * CELL`. `CLIPS` then maps `:idle`, `:walk` and `:run` to their row, frame order and fps — a clip is data, not a class.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 62-76 %}

### 5. Open the window, upload once and create the sprite

The window is created and capped at 60 fps, then `Texture.from_image(build_atlas)` moves the finished atlas to the GPU once and `smooth = false` keeps the pixel art crisp at 8×. The sprite is centred with an `origin` at the cell's middle and placed above the ground; `clip`, `time`, `speed`, `paused`, `flipped` and the `Clock` are the playback state.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 78-97 %}

### 6. Handle key events

`1`/`2`/`3` switch `clip`, space toggles `paused`, `+`/`-` step `speed` within `[0.25, 4.0]`, and `F` toggles `flipped` and mirrors the sprite. Escape and `'closed'` close the window.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 99-119 %}

### 7. Advance the frame

`clock.restart!` yields the frame delta, which accumulates into `time` scaled by `speed` unless paused. The frame index is `(time * fps).to_i % columns.length` for the active clip, and `sprite.texture_rect` is set to that cell — so changing `fps` or `speed` changes playback without touching the atlas.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 121-127 %}

### 8. Draw the scene and HUD

A wide ground `RectangleShape` is drawn first, then the sprite, then a HUD naming the clip, frame, fps, speed, pause and facing before `display!` presents the frame.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb 129-145 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Image` | Graphics | Atlas built pixel by pixel | `from_color`, `set_pixel` |
| `Texture` | Graphics | Uploaded atlas | `from_image`, `smooth=` |
| `Sprite` | Graphics | The animated drawable | `new`, `texture_rect=`, `origin=`, `position=`, `scale=` |
| `Clock` | System | Frame delta and clip timing | `new`, `restart!`, `elapsed_time` |
| `RectangleShape` | Graphics | The ground line | `new`, `position=`, `fill_color=` |
| `Window` | Window | Render surface | `new`, `poll_events!`, `clear!`, `draw`, `display!` |
| `Event` | Window | Clip/speed/flip keys | `type`, `code` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/sprite_animation/sprite_animation.rb` and run.

{% example ruby examples/graphics/sprite_animation/sprite_animation.rb %}

{: .note }
> Animation is just `texture_rect`: pick the cell by `column * cell_width, row * cell_height`. A negative `scale.x` (with a centred `origin`) flips the sprite. See [Graphics Assets]({% link book/graphics-assets.md %}) for the from-disk sheet, and [Tileset]({% link book/tileset.md %}) for the batched alternative.

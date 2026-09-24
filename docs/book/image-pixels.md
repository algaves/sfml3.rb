---
layout: default
title: Image Pixels
parent: "Part II — Basic"
grand_parent: Book
nav_order: 4
---

# Recipe: Image Pixels

`Image` — the CPU-side pixel buffer — and its trip to the GPU: build one from raw pixels, edit single pixels, blit with `copy_image`, key out a colour with `create_mask_from_color`, flip it, save it to a PNG and to memory, reload it, upload it with `Texture.from_image`, and read it back with `Texture#copy_to_image`.

> **Basic**

## Goal

An 800×600 window showing the same 64×64 gradient twice: on the left stretched 5× via a `Texture`, and on the right the blitted composite. `F`/`V` flip, `M` keys out magenta, `S` saves + reloads + reads back, `R` resets. Escape quits.

## How the code works

### 1. Build an Image from raw pixels

`image(width, height)` calls the block for every coordinate, appends a default alpha when only `[r, g, b]` is returned, packs each colour with `pack('C4')` into a binary `String`, and hands the buffer to `Image.from_pixels`. This is the one place raw bytes become a CPU-side `Image`.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 33-42 %}

### 2. The gradient and its magenta key block

`KEY_COLOR` is the exact magenta later removed by the mask, and `gradient` builds a 64×64 `Image` whose top-left 10×10 corner is that colour and whose remaining pixels form an `(x, y)`-driven gradient.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 45-57 %}

### 3. Open the window, write a pixel and blit a canvas

After the window exists, `set_pixel(0, 0, …)` overwrites exactly one pixel of the gradient with white. `Image.from_color` makes a flat 64×64 canvas, and `copy_image` blits a 16×16 sub-rectangle of the gradient into it at `[16, 16]` — all on the CPU.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 62-70 %}

### 4. Upload the Textures and prepare the reload

`Texture.from_image` moves each CPU image to the GPU once, and `smooth = false` keeps the pixels crisp when scaled. The `saved_path`/`info` pair backs the save readout, and `reload_texture` rebuilds the gradient `Texture` and clears `dirty` whenever the CPU image has changed.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 72-85 %}

### 5. Flip and colour-key from the keyboard

`F` and `V` call `flip_horizontally!` / `flip_vertically!`, and `M` calls `create_mask_from_color(KEY_COLOR, 0)` to turn every exact magenta pixel transparent. Each sets `dirty`, because editing the CPU `Image` does not update the GPU `Texture`.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 93-104 %}

### 6. Save, reload, round-trip and reset

`S` writes a PNG with `save_to_file`, reloads it with `Image.from_file`, encodes to memory with `save_to_memory('png')`, and reads the GPU texture back into a CPU image with `copy_to_image`, reporting all four in `info`. `R` rebuilds the gradient from scratch and marks it dirty.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 105-119 %}

### 7. Refresh the texture and draw both images

`reload_texture.call` runs first when `dirty`, then two `RectangleShape`s carry the textures through `texture`/`texture_rect` and are scaled up (5× and 3.75×) to compare the direct upload with the blitted canvas.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 124-144 %}

### 8. HUD and present

Two captions name the two columns, and a HUD reports the image size, the sampled `pixel(0, 0)` and the save/reload/round-trip results before `display!` presents the frame.

{% example ruby examples/graphics/image_pixels/image_pixels.rb 146-156 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Image` | Graphics | CPU pixel buffer being exercised | `from_pixels`, `from_color`, `from_file`, `set_pixel`, `pixel`, `copy_image`, `create_mask_from_color`, `flip_horizontally!`, `flip_vertically!`, `save_to_file`, `save_to_memory`, `size` |
| `Texture` | Graphics | GPU copy of an `Image` | `from_image`, `smooth=`, `copy_to_image` |
| `RectangleShape` | Graphics | Displays each texture, scaled | `new`, `texture=`, `texture_rect=`, `scale=`, `position=` |
| `Color` | Graphics | RGBA value for pixels and fills | `new`, `to_a`, `join` |
| `Rect` | Graphics | Source/destination rectangle for blits | `new` |
| `Window` | Window | Render surface | `new`, `poll_events!`, `clear!`, `draw`, `display!` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/image_pixels/image_pixels.rb` and run.

{% example ruby examples/graphics/image_pixels/image_pixels.rb %}

{: .note }
> `F` flips, `V` flips vertically, `M` applies the colour-key mask, `S` saves and reloads, `R` resets. `Image` lives in RAM and `Texture` on the GPU — batch pixel work on the `Image`, then upload once.

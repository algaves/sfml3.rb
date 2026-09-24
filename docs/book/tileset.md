---
layout: default
title: Tilemaps
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 5
---

# Recipe: Tilemaps

A tileset and a tile map. The 8-tile terrain atlas (grass, dirt, water, sand, stone, path, tree, flower) is drawn into an `Image` at runtime and uploaded once; a deterministic map is then rendered two ways — one `Sprite` per tile, or a single batched `VertexArray` of textured quads. `B` toggles the two, the arrow keys pan the `View` and the wheel zooms.

> **Intermediate**

## Goal

A 900×560 window onto a 40×28 world map with a lake, a sandy shore, a winding path, a stone patch and scattered trees and flowers. `B` switches between per-tile sprites and one batched draw call, the arrows pan, the wheel zooms, escape quits.

## How the code works

### 1. Setup helpers and tile constants

`Font.from_file` and `text` build the HUD, while `TILE`, `TILE_KINDS` and `ATLAS_WIDTH` fix the 16×16 cell size and the width of the strip. `COLORS` names every terrain colour once so the painter stays readable.

{% example ruby examples/graphics/tileset/tileset.rb 21-37 %}

### 2. The tile painter

`paint_tile` writes into an `Image` with `set_pixel`. Two locals do the work: `fill` covers the whole cell with a base colour, and `speckle` scatters deterministic pixels so each tile has texture.

{% example ruby examples/graphics/tileset/tileset.rb 40-53 %}

### 3. One branch per terrain kind

The `case` paints each of the eight kinds: plain fills and speckle for grass, dirt, sand and path, horizontal ripples for water, cracks for stone, and simple rectangles for the tree and flowers. Every branch writes only inside its own 16×16 cell.

{% example ruby examples/graphics/tileset/tileset.rb 55-89 %}

### 4. Build and upload the atlas

`build_tileset` allocates a transparent `Image` and calls `paint_tile` once per kind in `TILE_ORDER`. `Texture.from_image` uploads it once and `smooth = false` keeps the pixels crisp when the view zooms.

{% example ruby examples/graphics/tileset/tileset.rb 91-97 %}

### 5. Generate a deterministic map

`build_map` fills a 40×28 array of tile indices from maths rather than randomness: an ellipse paints the lake and its sand ring, a sine paints the path, a coordinate range the stone patch, and a hash of `(x, y)` scatters trees and flowers, so any run is identical.

{% example ruby examples/graphics/tileset/tileset.rb 103-123 %}

### 6. Batch the map into quads

`build_batch` emits four `Vertex`es per tile — world position plus pixel `tex_coords` from the atlas — into a `VertexArray` whose `primitive` is `:quads`. Bound to the atlas through a `RenderState`, the whole map becomes one draw call.

{% example ruby examples/graphics/tileset/tileset.rb 139-157 %}

### 7. Events, pan and zoom

`poll_events!` handles close, toggles `batched` on `B` and zooms the `View` on the mouse wheel. Held arrow keys then move `view.center`, and `window.view = view` applies the camera to the map.

{% example ruby examples/graphics/tileset/tileset.rb 168-191 %}

### 8. Render and HUD

The batch or the per-tile `Sprite` loop draws the map under the view. `window.view = window.default_view` restores the pixel view so the HUD text stays fixed, and `display!` presents the frame.

{% example ruby examples/graphics/tileset/tileset.rb 193-219 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Image` | Graphics | Tileset built at runtime | `from_color`, `set_pixel` |
| `Texture` | Graphics | Uploaded atlas | `from_image`, `smooth=` |
| `Sprite` | Graphics | Per-tile renderer | `new`, `texture_rect=`, `position=`, `origin=` |
| `VertexArray` / `Vertex` | Graphics | Batched renderer | `new`, `primitive=`, `append` |
| `RenderState` | Graphics | Binds the atlas for the batch | `new`, `texture=` |
| `View` | Graphics | Camera: pan and zoom | `from_rect`, `center`, `center=`, `zoom`, `size` |
| `Rect` / `Color` | Graphics | View rect and image fills | `new` |
| `Window` / `Keyboard` | Window | Surface and held keys | `new`, `view=`, `default_view`, `key_pressed?` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/tileset/tileset.rb` and run.

{% example ruby examples/graphics/tileset/tileset.rb %}

{: .note }
> A quad is four `Vertex`es carrying world positions plus pixel `tex_coords`, drawn with the atlas bound through `RenderState#texture` — one draw call for the whole map. The per-tile `Sprite` mode is the simpler, slower alternative.

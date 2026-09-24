---
layout: default
title: Sprites & Textures
parent: Learn
nav_order: 5
---

# Tutorial 5: Sprites & Textures

Textures live on the GPU, sprites draw them, images edit pixels on the CPU. Get this pipeline right and everything visual follows.

## Load once, draw many

```ruby
require 'sfml'

texture = SF::Graphics::Texture.from_file('player.png')
texture.smooth = true # filtered scaling

sprite = SF::Graphics::Sprite.new(texture)
sprite.position = [100, 50]

SF::Graphics::RenderWindow.open(SF::Window::VideoMode[800, 600], 'Sprites') do |window|
  while window.open?
    window.poll_events! { |e| window.close! if e.closed? }
    window.render!(clear_color: SF::Graphics::Color::BLACK) { |t| t.draw(sprite) }
  end
end
```

{: .warning }
> Create the window **before** the texture (textures need a GL context). Keep the `Texture` object alive as long as any sprite uses it — sprites hold a pointer, not a copy.

## Sprite sheets via texture_rect

```ruby
# 4 frames of 32x32 in one row:
frame = 0
clock = SF::System::Clock.new
while window.open?
  if clock.elapsed_time.as_seconds >= 0.15
    frame = (frame + 1) % 4
    sprite.texture_rect = SF::Graphics::Rect[frame * 32, 0, 32, 32]
    clock.restart!
  end
end
```

Tint with `sprite.color = SF::Graphics::Color[255, 128, 128]` (multiplier, white = neutral).

## CPU pixels with Image

```ruby
img = SF::Graphics::Image.from_file('in.png')
img.set_pixel(0, 0, SF::Graphics::Color::RED)
img.create_mask_from_color(SF::Graphics::Color[255, 0, 255], 0) # color-key
img.save_to_file('out.png')
tex = SF::Graphics::Texture.from_image(img)
```

Batch pixel work on `Image`, upload once. Per-frame `set_pixel` + re-upload is slow.

## Smoothing, tiling, mipmaps

```ruby
tex.smooth = true     # bilinear filtering (scaled sprites)
tex.repeated = true   # tile when texture_rect exceeds bounds
tex.generate_mipmap   # after update_from_* if minified
puts SF::Graphics::Texture.maximum_size
```

## Worked examples

- [Graphics Assets]({% link book/graphics-assets.md %}) — a from-disk sheet, runtime textures and a `RenderTexture`.
- [Image Pixels]({% link book/image-pixels.md %}) — `Image` editing, blitting, colour-keying and save/reload.
- [Sprite Animation]({% link book/sprite-animation.md %}) — named clips driven by `texture_rect`.
- [Tileset & Tile Map]({% link book/tileset.md %}) — many tiles drawn from one atlas, batched.
- [Graphics: Sprites, textures & images]({% link api/graphics.md %}#sprites-textures--images) — the full method tables.

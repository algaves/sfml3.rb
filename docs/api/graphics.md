---
layout: default
title: Graphics
parent: API Reference
nav_order: 1
---

# Graphics API

Core rendering classes: windows, textures, sprites, shapes, text, views, shaders, and render targets.

## Key classes

| Class | Purpose |
|---|---|
| `SF::Graphics::RenderWindow` | Window with 2D drawing surface |
| `SF::Graphics::RenderTexture` | Offscreen render target |
| `SF::Graphics::RenderTarget` | Common render target interface |
| `SF::Graphics::Sprite` | Drawable textured quad |
| `SF::Graphics::Texture` | Image texture resource |
| `SF::Graphics::Image` | CPU-side image operations |
| `SF::Graphics::Shape` | Base shape drawable |
| `SF::Graphics::CircleShape` | Circle/ellipse shape |
| `SF::Graphics::RectangleShape` | Axis-aligned rectangle |
| `SF::Graphics::ConvexShape` | Convex polygon |
| `SF::Graphics::VertexArray` | Batch of vertices |
| `SF::Graphics::Text` | Text rendering |
| `SF::Graphics::Font` | Font resource |
| `SF::Graphics::Shader` | GLSL shader program |
| `SF::Graphics::View` | Camera/view transform |
| `SF::Graphics::Transform` | 2D affine transform |
| `SF::Graphics::Transformable` | Mixin for position/rotation/scale |
| `SF::Graphics::Color` | RGBA color |
| `SF::Graphics::Rect` / `SF::Graphics::FloatRect` | Axis-aligned rectangles |

See RBS signatures under `sig/graphics/` and YARD C comments in `ext/graphics/` for method-level details.

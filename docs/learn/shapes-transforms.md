---
layout: default
title: Shapes & Transforms
parent: Learn
nav_order: 6
---

# Tutorial 6: Shapes & Transforms

Shapes are transformable drawables (`position`, `rotation`, `scale`, `origin`). Master `origin` and the global-bounds split and layout becomes predictable.

## The four shapes

```ruby
require 'sfml'

circle = SF::Graphics::CircleShape.new(50)
circle.point_count = 64
circle.fill_color = SF::Graphics::Color::CYAN

box = SF::Graphics::RectangleShape.new([200, 100])
box.position = [50, 50]

tri = SF::Graphics::ConvexShape[SF::System::Vector2[0, 0], SF::System::Vector2[64, 0], SF::System::Vector2[32, 48]]

class Star < SF::Graphics::Shape
  def initialize(points)
    super()
    @pts = points
    update! # required!
  end

  def point_count = @pts.size
  def point(i) = @pts[i]
end
```

- `CircleShape`: raise `point_count` for large smooth circles.
- `RectangleShape[x, y, w, h]` sets size semantics — position separately.
- `ConvexShape`: set `point_count` before `set_point`; must stay convex.
- Custom `Shape`: call `update!` after construction and after any point change.

## Origin is the pivot

```ruby
box.origin = [100, 50] # center of a 200x100 box
box.rotation = 45      # now spins about its center
```

Default `origin` is `(0,0)` (top-left). For centered rotation set `origin = size / 2`.

## Local vs global bounds

```ruby
local = box.local_bounds   # untransformed Rect
world = box.global_bounds  # after position/rotation/scale
puts world.contains?([mouse_x, mouse_y])
```

Use `global_bounds` for hit-testing, `local_bounds` for layout.

## Transforms compose

```ruby
t = SF::Graphics::Transform.identity
t = t.translate([10, 20]).rotate(45).scale([2, 2])
state = SF::Graphics::RenderState.new
state.transform = t
window.draw(box, state)
```

`a * b` means `a` applied after `b`. Bang variants (`translate!`) mutate; plain variants return new.

## Worked examples

- [Shape]({% link book/drawing-shapes.md %}) — the built-in shapes, styling and texturing.
- [Transformable]({% link book/shape-transforms.md %}) — origin pivots, bounds and composed `Transform`s.
- [Custom Shape]({% link book/custom-shape.md %}) — subclassing `Shape` to supply your own geometry.
- [Vertex Arrays]({% link book/vertex-arrays.md %}) and [Particle Systems]({% link book/particles.md %}) — raw vertices and a system built on them.
- [Drawable]({% link book/custom-drawable.md %}) — the `Drawable` mixin in plain Ruby.
- [Graphics: Shapes]({% link api/graphics.md %}#shapes) and [Transforms]({% link api/graphics.md %}#transforms) — the full method tables.

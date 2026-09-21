# Examples

Both examples are built from the same five components: a **Window**, the
**Events** it polls, the **Transformable** mixin, **Drawable**-based objects,
and the **primitive shapes**. They need a real display (SFML aborts rather than
raises when there is none); headless machines can run them under `xvfb-run -a`.
Neither is part of the test suite.

Run them from the repository root. `bundle exec` keeps the checkout's freshly
built extension from being shadowed by an installed gem; with the gem installed
instead, plain `ruby examples/hello_shapes.rb` works.

```
bundle exec ruby -Ilib examples/hello_shapes.rb
xvfb-run -a bundle exec ruby -Ilib examples/bouncing_shapes.rb
```

| Component          | What it does                          | Key API (see `sig/`)                                                                  |
| ------------------ | ------------------------------------- | ------------------------------------------------------------------------------------- |
| Window             | Owns the OS window and render loop    | `Window.new(VideoMode.new(w, h, bpp), title)`, `frame_rate=`, `clear!`, `display!`, `open?`, `close!`, `render!` |
| Events             | Input and window notifications        | `Event.new`, `poll_events! { \|event\| }`, `event.type`, `event.closed?`, `event.key_pressed?`, `event.code`, `event.mouse_button`, `event.mouse_move` |
| Transformable      | Position, rotation, scale, origin     | `position=`, `move`, `rotate`, `scale=`, `origin=` (a mixin included by every drawable) |
| Drawable objects   | Anything `window.draw` accepts        | `Drawable` mixin; `Shape`/`Sprite`/`Text`/`VertexArray` include it                    |
| Primitive shapes   | The built-in filled shapes            | `CircleShape`, `RectangleShape`, `ConvexShape#set_point`, `fill_color=`, `outline_*`    |

A new example built on these components only needs to fill in the loop between
`clear!` and `display!`:

```ruby
window = Window.new(VideoMode.new(640, 480, 32), 'title')

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed' then window.close!
    when 'key-pressed' then window.close! if event.code == :escape
    end
  end

  # move / rotate / scale the transformables...
  window.clear!([24, 24, 34, 255]) # [r, g, b, a], 0-255
  # window.draw(shape)...
  window.display!
end
```

* `hello_shapes.rb` -- the minimal walkthrough: each component used once.
* `bouncing_shapes.rb` -- the same components reinterpreted as an interactive
  app (mouse grabs/drags shapes, the wheel rescales them, space spawns, escape
  quits).
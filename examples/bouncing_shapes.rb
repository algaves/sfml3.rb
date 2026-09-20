# frozen_string_literal: true

# The same five components as hello_shapes.rb -- Window, Events, Transformable,
# Drawable objects, primitive shapes -- reinterpreted as an interactive app:
# shapes bounce off the window edges, the mouse grabs and drags a shape, the
# wheel rescales them, space spawns a fresh one and Escape quits.
#
# Run from the repository root with a display (Bundler so the checkout's build
# wins over an installed gem): `bundle exec ruby -Ilib examples/bouncing_shapes.rb`
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SFML

Bouncer = Struct.new(:shape, :vx, :vy)
MARGIN = 20

def random_color
  Array.new(3) { rand 255 } + [255]
end

def new_shape
  [CircleShape.new(rand(20..40)),
   RectangleShape.new([rand(40..80), rand(40..80)]),
   ConvexShape.new(3)].sample.tap do |shape|
    shape.position = [320, 240]
    shape.origin = [20, 20]
    shape.fill_color = random_color
  end
end

def bounce(axis, velocity, radius, limit)
  if axis + radius > limit
    [limit - radius, -velocity.abs]
  elsif (axis - radius).negative?
    [radius, velocity.abs]
  else
    [axis, velocity]
  end
end

def near?(bouncer, point)
  position = bouncer.shape.position
  (position.x - point.x).abs < MARGIN && (position.y - point.y).abs < MARGIN
end

def spawn(bouncers, window_size)
  shape = new_shape
  shape.position = [window_size.x / 2, window_size.y / 2]
  bouncers << Bouncer.new(shape, rand(-3..4), rand(-3..4))
end

window = Window.new(VideoMode.new(640, 480, 32), 'SFML bouncing shapes')
window.frame_rate = 60

bouncers = Array.new(4) do |index|
  shape = new_shape
  shape.position = [100 + (index * 130), 240]
  Bouncer.new(shape, rand(-3..4), rand(-3..4).nonzero? || 2)
end

held = nil
grab_offset = [0, 0]
event = Event.new

while window.is_open?
  while window.poll_event!(event)
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      if event.key[:code] == :escape
        window.close!
      elsif event.key[:code] == :space
        spawn(bouncers, window.size)
      end
    when 'mouse-button-pressed'
      if event.mouse_button[:button] == :left
        held = bouncers.find { |bouncer| near?(bouncer, event.mouse_button[:position]) }
        if held
          position = held.shape.position
          point = event.mouse_button[:position]
          grab_offset = [position.x - point.x, position.y - point.y]
        end
      end
    when 'mouse-button-released'
      held = nil
    when 'mouse-moved'
      if held
        held.shape.position = [event.mouse_move.x + grab_offset[0],
                               event.mouse_move.y + grab_offset[1]]
      end
    when 'mouse-wheel-scrolled'
      factor = event.mouse_wheel_scroll[:delta].positive? ? 1.1 : 0.9
      bouncers.each { |bouncer| bouncer.shape.scale!(factor) }
    end
  end

  size = window.size

  bouncers.each do |bouncer|
    next if bouncer.equal?(held) # the cursor owns it until released

    shape = bouncer.shape
    position = shape.position
    x, bouncer.vx = bounce(position.x + bouncer.vx, bouncer.vx, MARGIN, size.x)
    y, bouncer.vy = bounce(position.y + bouncer.vy, bouncer.vy, MARGIN, size.y)
    shape.position = [x, y]
    shape.rotate(1)
  end

  window.clear([24, 24, 34, 255])
  bouncers.each { |bouncer| window.draw(bouncer.shape) }
  window.display
end

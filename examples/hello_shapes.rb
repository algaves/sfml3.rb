# frozen_string_literal: true

# The five building blocks every example is built from: a Window, the events it
# polls, the Transformable mixin, Drawable-based objects, and the primitive
# shapes. Nothing else is used -- no clock, views, or textures -- so each
# component is easy to see in isolation.
#
# Run from the repository root with a display (Bundler so the checkout's build
# wins over an installed gem): `bundle exec ruby -Ilib examples/hello_shapes.rb`
# (headless: prefix `xvfb-run -a`). Close the window or press Escape to quit.

require 'sfml'
include SFML

window = Window.new(VideoMode.new(640, 480, 32), 'SFML hello shapes')
window.frame_rate = 60

circle = CircleShape.new(60)
circle.position = [100, 100]
circle.origin = [60, 60] # rotate about the centre, not the corner
circle.fill_color = [230, 90, 90, 255]

square = RectangleShape.new([140, 140])
square.position = [300, 100]
square.origin = [70, 70]
square.fill_color = [90, 170, 230, 255]

triangle = ConvexShape.new(3)
triangle.set_point(0, [0, 0])
triangle.set_point(1, [120, 0])
triangle.set_point(2, [60, 100])
triangle.position = [120, 300]
triangle.origin = [60, 50]
triangle.outline_thickness = 4
triangle.outline_color = [255, 255, 255, 255]

shapes = [circle, square, triangle]
event = Event.new

while window.is_open?
  while window.poll_event!(event)
    case event.type
    when 'closed' then window.close!
    when 'key-pressed' then window.close! if event.key[:code] == :escape
    end
  end

  circle.rotate(1)
  square.rotate(-1)
  triangle.position = [circle.position.x + 260, circle.position.y]

  window.clear([28, 28, 38, 255])
  shapes.each { |shape| window.draw(shape) }
  window.display
end

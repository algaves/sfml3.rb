# frozen_string_literal: true

# The Transformable mixin, which every drawable includes: position, rotation,
# scale and origin. R switches the rectangle between corner-pivot and
# centre-pivot, the arrow keys move it, the wheel scales it and it spins while
# space is off. The yellow box is its global (transformed) bounds and the red
# dot is the origin; hover the rectangle to see global_bounds#contains? hit-test
# it. The bottom row shows a Transform composed and applied through a
# RenderState.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/shapes_transforms/shapes_transforms.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

WIDTH = 800
HEIGHT = 600

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML transformable: origin, bounds, transforms')
window.frame_rate = 60

box = RectangleShape.new([200, 120])
box.position = [300, 260]
box.fill_color = [90, 170, 230, 255]
box.outline_thickness = 3
box.outline_color = [255, 255, 255, 255]

centered = false
spinning = true
hover = false
frame = 0

# A shape whose transform is supplied per draw through a RenderState.
wedge = ConvexShape.new(3)
wedge.set_point(0, [0, -18])
wedge.set_point(1, [16, 12])
wedge.set_point(2, [-16, 12])

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :r
        centered = !centered
        box.origin = centered ? box.size : [0.0, 0.0]
      when :Space
        spinning = !spinning
      end
    when 'mouse-wheel-scrolled'
      factor = event.mouse_wheel_scroll[:delta].positive? ? 1.1 : 0.9
      box.scale!(factor)
    when 'mouse-moved'
      hover = box.global_bounds.contains?([event.mouse_move.x, event.mouse_move.y])
    end
  end

  box.move([-4, 0]) if Keyboard.key_pressed?(:Left)
  box.move([4, 0]) if Keyboard.key_pressed?(:Right)
  box.move([0, -4]) if Keyboard.key_pressed?(:Up)
  box.move([0, 4]) if Keyboard.key_pressed?(:Down)
  box.rotate(1) if spinning

  window.clear!([22, 24, 34, 255])

  # The shape itself + its transformed bounds outline.
  window.draw(box)
  bounds = box.global_bounds
  outline = RectangleShape.new(bounds.size)
  outline.position = [bounds.left, bounds.top]
  outline.fill_color = [255, 255, 255, 0]
  outline.outline_thickness = 2
  outline.outline_color = hover ? [255, 120, 120, 255] : [255, 210, 90, 255]
  window.draw(outline)

  # The origin (the pivot for rotation/scale) as a small dot.
  pivot = CircleShape.new(5)
  origin = box.origin
  position = box.position
  pivot.position = [position.x + origin.x, position.y + origin.y]
  pivot.fill_color = [255, 90, 90, 255]
  window.draw(pivot)

  # Transform composition: the same wedge at three angles, each drawn through a
  # RenderState whose transform is built with translate / rotate / scale.
  [0, 60, 120].each_with_index do |angle, index|
    transform = Transform.identity
                         .translate([140 + (index * 110), 500])
                         .rotate(angle + (frame * 1.5))
                         .scale([1.0 + (index * 0.4), 1.0 + (index * 0.4)])
    state = RenderState.new
    state.transform = transform
    window.draw(wedge, state)
  end

  window.draw(text(
                "origin #{origin.x.to_i},#{origin.y.to_i}   rotation #{box.rotation.round}\u00b0   " \
                "scale #{box.scale.x.round(2)}   pivot: #{centered ? 'centre' : 'corner'}   hover: #{hover}\n" \
                "local_bounds #{box.local_bounds.size.x.to_i}x#{box.local_bounds.size.y.to_i}   " \
                "global_bounds #{bounds.width.to_i}x#{bounds.height.to_i} at #{bounds.left.to_i},#{bounds.top.to_i}\n" \
                'R pivot, arrows move, wheel scale, space spin, escape quits',
                size: 15, position: [26, 30]
              ))
  window.display!

  frame += 1
end

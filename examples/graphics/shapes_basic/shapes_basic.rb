# frozen_string_literal: true

# The three built-in shapes, side by side: CircleShape, RectangleShape and
# ConvexShape. Shows the styling they share (fill, outline thickness and
# colour), the circle's `point_count` tessellation, the origin pivot, and how a
# shape can be textured instead of filled. Press 1/2/3 to select a shape
# (its global bounds are outlined), +/- to change the circle tessellation,
# T to toggle the outlines, space to texture the rectangle, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/shapes_basic/shapes_basic.rb
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

# Builds a CPU Image from a block returning [r, g, b] or [r, g, b, a].
def image(width, height)
  pixels = String.new(encoding: Encoding::BINARY)
  height.times do |y|
    width.times do |x|
      color = yield(x, y)
      color += [255] if color.size == 3
      pixels << color.pack('C4')
    end
  end
  Image.from_pixels([width, height], pixels)
end

# A small checkerboard Texture, tiled by `repeated`.
def checker_texture(size, cell)
  texture = Texture.from_image(image(size, size) do |x, y|
    ((x / cell) + (y / cell)).even? ? [62, 92, 142, 255] : [92, 132, 192, 255]
  end)
  texture.repeated = true
  texture
end

WIDTH = 900
HEIGHT = 560

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML shapes: circle, rectangle, convex')
window.frame_rate = 60

circle = CircleShape.new(70)
circle.point_count = 64
circle.position = [170, 200]
circle.origin = [70, 70] # spin about the centre, not the top-left corner
circle.fill_color = [230, 90, 90, 255]
circle.outline_thickness = 4
circle.outline_color = [255, 255, 255, 255]

rectangle = RectangleShape.new([180, 120])
rectangle.position = [460, 200]
rectangle.origin = [90, 60]
rectangle.fill_color = [90, 170, 230, 255]
rectangle.outline_thickness = 4
rectangle.outline_color = [255, 255, 255, 255]

# A regular hexagon: points are laid out on a circle of radius 80, and the
# convex polygon must stay convex.
SIDES = 6
hexagon = ConvexShape.new(SIDES)
SIDES.times do |index|
  angle = index * 2 * Math::PI / SIDES
  hexagon.set_point(index, [Math.cos(angle) * 80, Math.sin(angle) * 80])
end
hexagon.position = [750, 200]
hexagon.fill_color = [110, 200, 120, 255]
hexagon.outline_thickness = 4
hexagon.outline_color = [255, 255, 255, 255]

shapes = [circle, rectangle, hexagon]
NAMES = %w[CircleShape RectangleShape ConvexShape].freeze

checker = checker_texture(32, 8)
rectangle.texture = checker
rectangle.texture_rect = [0, 0, 180, 120]

selected = 0
outline = true
textured = true

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :num1 then selected = 0
      when :num2 then selected = 1
      when :num3 then selected = 2
      when :t
        outline = !outline
        shapes.each { |shape| shape.outline_thickness = outline ? 4 : 0 }
      when :Space
        textured = !textured
        rectangle.texture = textured ? checker : nil
        rectangle.texture_rect = [0, 0, 180, 120] if textured
      when :Equal, :Add
        circle.point_count = [circle.point_count + 4, 64].min
      when :Hyphen, :Subtract
        circle.point_count = [circle.point_count - 4, 8].max
      end
    end
  end

  shapes.each { |shape| shape.rotate(1) }

  window.clear!([24, 26, 36, 255])

  shapes.each { |shape| window.draw(shape) }

  # Outline the selected shape's transformed (global) bounds.
  bounds = shapes[selected].global_bounds
  marker = RectangleShape.new(bounds.size)
  marker.position = [bounds.left, bounds.top]
  marker.fill_color = [255, 255, 255, 0]
  marker.outline_thickness = 2
  marker.outline_color = [255, 210, 90, 255]
  window.draw(marker)

  labels = NAMES.each_with_index.map { |name, index| index == selected ? "[#{name}]" : " #{name} " }.join
  window.draw(text("shapes: #{labels}", size: 18, position: [30, 430]))
  window.draw(text(
                "circle points: #{circle.point_count}   outlines: #{outline}   rect textured: #{textured}\n" \
                '1/2/3 select, +/- tessellation, T outlines, space texture, escape quits',
                size: 15, position: [30, 465]
              ))
  window.display!

end

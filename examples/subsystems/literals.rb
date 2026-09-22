# frozen_string_literal: true

# The Ruby literals this binding accepts. SFML's C++ `sf::` user-defined
# literals (e.g. "text"_s) do not exist here -- CSFML takes plain C strings --
# but every geometry and colour argument takes an Array where SFML would take a
# struct. This example builds the same objects from literals, constructors and
# constants, and prints their round-trips. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/literals.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

color_a = Color.new([230, 90, 90, 255])          # [r, g, b, a] array
color_b = Color.new(90, 170, 230)                # r, g, b (alpha defaults to 255)
color_c = Color.from_rgb(0x33AA66)               # packed 0xRRGGBB
color_d = Color::YELLOW                          # named constant

position = [120, 120]                            # [x, y]
size = [90, 70]                                  # [width, height]
rect = Rect.new([10, 20, 30, 40])                # [left, top, width, height]
point = Vector3.new([1.0, 2.0, 3.0])             # [x, y, z]
sum = Vector2.new([10, 20]) + [5, 5]             # arithmetic against an array

lines = [
  "Color.new([230, 90, 90, 255])  -> to_a #{color_a.to_a.inspect}",
  "Color.new(90, 170, 230)         -> #{color_b}",
  "Color.from_rgb(0x33AA66)        -> #{color_c}",
  "Color::YELLOW                   -> #{color_d}",
  '',
  "position = [120, 120]           -> Vector2#to_a #{Vector2.new(position).to_a.inspect}",
  "size = [90, 70]                 -> Vector2#to_a #{Vector2.new(size).to_a.inspect}",
  "Rect.new([10, 20, 30, 40])      -> to_a #{rect.to_a.inspect}",
  "Vector3.new([1.0, 2.0, 3.0])    -> to_a #{point.to_a.inspect}",
  "Vector2.new([10, 20]) + [5, 5]  -> #{sum}",
  '',
  "SF::System::Time.seconds(1.5)         -> #{SF::System::Time.seconds(1.5)}  " \
  "(#{SF::System::Time.seconds(1.5).as_milliseconds} ms)",
  "SF::System::Time.milliseconds(250)    -> #{SF::System::Time.milliseconds(250)}  " \
  "(#{SF::System::Time.milliseconds(250).to_f} s)"
]

window = Window.new(VideoMode.new(720, 520, 32), 'SFML literals')
window.frame_rate = 60
frame = 0

triangle = ConvexShape.new(3)
triangle.set_point(0, [0, 0])
triangle.set_point(1, [80, 0])
triangle.set_point(2, [40, 70])

shapes = [
  [CircleShape.new(40), color_a, [40, 40]],
  [RectangleShape.new(size), color_b, [120, 40]],
  [triangle, color_c, [240, 40]]
]

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    end
  end

  window.clear!([22, 26, 34, 255])

  shapes.each do |shape, color, corner|
    shape.fill_color = color
    shape.position = corner
    window.draw(shape)
  end

  lines.each_with_index do |line, index|
    window.draw(ExampleSupport.text(line, size: 15, position: [24, 170 + (index * 20)]))
  end
  window.draw(ExampleSupport.text('Literals accepted anywhere SFML takes a struct. Escape quits.',
                                  size: 16, position: [24, 495]))
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

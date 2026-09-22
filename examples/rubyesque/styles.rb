# frozen_string_literal: true

# Rubyesque (Matz-like): the positional constructors and the window loop. The
# window is built from VideoMode[...] and Style::DEFAULT, the scene from
# Vector2/Color and the shape forms (CircleShape[r, [x, y]],
# RectangleShape[x, y, w, h], ConvexShape[[x0, y0], ...]), and open! runs the
# loop while poll_event! drains it in block form. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/rubyesque/styles.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

circle = CircleShape[44, [170, 220]]
circle.origin = Vector2[44, 44]
circle.fill_color = Color[230, 90, 90]

square = RectangleShape[360, 130, 120, 90]
square.origin = Vector2[60, 45]
square.fill_color = Color[90, 170, 230]

triangle = ConvexShape[[460, 300], [600, 300], [530, 200]]
triangle.fill_color = Color[120, 220, 140]

shapes = [circle, square, triangle]
frame = 0

window = Window.new(VideoMode[720, 420, 32], 'SFML rubyesque: constructors', Style::DEFAULT)
window.open! do
  window.poll_event! do |event|
    window.close! if event.closed?
    window.close! if event.key_pressed? && event.code == :escape
  end

  shapes.each { |shape| shape.rotate(1) }

  window.render!(clear_color: Color[24, 26, 34]) do |target|
    shapes.each { |shape| target.draw(shape) }
    target.draw(ExampleSupport.text(
                  "VideoMode[720, 420, 32]   Style::DEFAULT\n" \
                  "CircleShape[r, [x, y]]    RectangleShape[x, y, w, h]\n" \
                  "ConvexShape[[x0, y0], ...]   Color[r, g, b]   Vector2[x, y]\n\n" \
                  'rotate about Vector2 origins, escape quits',
                  size: 15, position: [16, 12]
                ))
  end

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

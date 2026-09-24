# frozen_string_literal: true

# Scrolling is a camera you move yourself: pan a View across a world larger
# than the window. Here the arrow keys pan, `A` toggles an auto-scroll that
# loops back to the left edge, and the wheel zooms. The HUD reads the camera's
# live centre and size, so you can see the view do the work. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/views_scrolling/views_scrolling.rb
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

WIDTH = 900
HEIGHT = 560
WORLD_WIDTH = 2400
WORLD_HEIGHT = 1600

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML views: scrolling a large world')
window.frame_rate = 60

# A grid plus a numbered column every 400px, so horizontal motion is obvious.
grid = VertexArray.new
grid.primitive = :lines
(0..WORLD_WIDTH).step(100) do |x|
  grid.append(Vertex.new([x, 0], [44, 50, 70, 255]))
  grid.append(Vertex.new([x, WORLD_HEIGHT], [44, 50, 70, 255]))
end
(0..WORLD_HEIGHT).step(100) do |y|
  grid.append(Vertex.new([0, y], [44, 50, 70, 255]))
  grid.append(Vertex.new([WORLD_WIDTH, y], [44, 50, 70, 255]))
end

markers = (0..5).map do |i|
  shape = RectangleShape.new([6, WORLD_HEIGHT])
  shape.position = [i * 400, 0]
  shape.fill_color = [90, 170, 230, 140]
  shape
end

finish = RectangleShape.new([12, WORLD_HEIGHT])
finish.position = [WORLD_WIDTH - 12, 0]
finish.fill_color = [255, 210, 90, 255]

camera = View.from_rect(Rect.new(0, 0, WIDTH, HEIGHT))
camera.center = [WIDTH / 2, HEIGHT / 2]
base_size = camera.size

auto = false

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :a then auto = !auto
      end
    when 'mouse-wheel-scrolled'
      camera.zoom(event.mouse_wheel_scroll[:delta].positive? ? 0.9 : 1.1)
    end
  end

  center = camera.center
  dx = 0.0
  dy = 0.0
  pan = 320.0 / 60
  dx -= pan if Keyboard.key_pressed?(:Left)
  dx += pan if Keyboard.key_pressed?(:Right)
  dy -= pan if Keyboard.key_pressed?(:Up)
  dy += pan if Keyboard.key_pressed?(:Down)
  dx += (160.0 / 60) if auto

  # Wrap around the world horizontally so the scroll can run forever.
  x = center.x + dx
  x = camera.size.x / 2 if x > WORLD_WIDTH - (camera.size.x / 2)
  x = WORLD_WIDTH - (camera.size.x / 2) if x < camera.size.x / 2
  y = (center.y + dy).clamp(camera.size.y / 2, WORLD_HEIGHT - (camera.size.y / 2))
  camera.center = [x, y]

  window.clear!([16, 18, 28, 255])
  window.view = camera
  window.draw(grid)
  markers.each { |marker| window.draw(marker) }
  window.draw(finish)

  window.view = window.default_view
  size = camera.size
  zoom = (base_size.x / size.x).round(2)
  window.draw(text(
                "centre #{camera.center.x.to_i},#{camera.center.y.to_i}   " \
                "view #{size.x.to_i}x#{size.y.to_i}   zoom #{zoom}x   " \
                "auto-scroll #{auto ? 'on' : 'off'}\n" \
                'arrows pan, A auto-scroll, wheel zoom, escape quits',
                size: 15
              ))
  window.display!
end

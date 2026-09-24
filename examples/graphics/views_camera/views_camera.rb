# frozen_string_literal: true

# A View is a camera: a rectangle onto the world that decides which part is
# drawn and where on the target it lands. This recipe uses one to follow a
# player through a world larger than the window, clamping the camera to the
# world edges so the void never shows. The arrow keys move the player, escape
# quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/views_camera/views_camera.rb
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
WORLD_WIDTH = 1600
WORLD_HEIGHT = 1000

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML views: a following camera')
window.frame_rate = 60

player = CircleShape.new(18)
player.origin = [18, 18]
player.fill_color = [255, 210, 90, 255]
player.position = [WORLD_WIDTH / 2, WORLD_HEIGHT / 2]

# Landmarks make movement visible at a glance.
LANDMARKS = [
  [120, 120, [230, 90, 90, 255]], [1400, 200, [90, 200, 130, 255]],
  [260, 800, [90, 170, 230, 255]], [1300, 820, [180, 110, 235, 255]]
].freeze

landmarks = LANDMARKS.map do |x, y, color|
  shape = RectangleShape.new([120, 120])
  shape.position = [x, y]
  shape.fill_color = color
  shape
end

# A grid drawn from one VertexArray, shared by every recipe in this section.
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

# The camera starts the size of the window; its centre picks what it looks at.
camera = View.from_rect(Rect.new(0, 0, WIDTH, HEIGHT))

# Keeps the camera's view inside the world instead of showing the void.
def clamp_center(position, view_size)
  [position.x.clamp(view_size.x / 2, WORLD_WIDTH - (view_size.x / 2)),
   position.y.clamp(view_size.y / 2, WORLD_HEIGHT - (view_size.y / 2))]
end

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    end
  end

  speed = 5
  player.move([-speed, 0]) if Keyboard.key_pressed?(:Left)
  player.move([speed, 0]) if Keyboard.key_pressed?(:Right)
  player.move([0, -speed]) if Keyboard.key_pressed?(:Up)
  player.move([0, speed]) if Keyboard.key_pressed?(:Down)
  position = player.position
  player.position = [position.x.clamp(0, WORLD_WIDTH), position.y.clamp(0, WORLD_HEIGHT)]

  # The camera tracks the player, clamped so the edge of the world stays put.
  camera.center = clamp_center(player.position, camera.size)

  window.clear!([16, 18, 28, 255])
  window.view = camera
  window.draw(grid)
  landmarks.each { |shape| window.draw(shape) }
  window.draw(player)

  # Back to pixels for the HUD: it must stay fixed while the world scrolls.
  window.view = window.default_view
  window.draw(text(
                "camera centre #{camera.center.x.to_i},#{camera.center.y.to_i}   " \
                "view #{camera.size.x.to_i}x#{camera.size.y.to_i}\n" \
                'arrows move the player, escape quits',
                size: 15
              ))
  window.display!
end

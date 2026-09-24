# frozen_string_literal: true

# A View is a camera; its viewport is a normalised (0-1) rectangle saying where
# on the target that camera's image lands. Two views with half-window viewports
# split the screen into two independent cameras: here the left follows a player
# while the right stays a fixed overview of the whole world. The arrow keys move
# the player, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/views_split/views_split.rb
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
HALF = WIDTH / 2
WORLD_WIDTH = 1600
WORLD_HEIGHT = 1000

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML views: split-screen viewports')
window.frame_rate = 60

player = CircleShape.new(16)
player.origin = [16, 16]
player.fill_color = [255, 210, 90, 255]
player.position = [WORLD_WIDTH / 2, WORLD_HEIGHT / 2]

landmarks = [
  [120, 120, [230, 90, 90, 255]], [1400, 200, [90, 200, 130, 255]],
  [260, 800, [90, 170, 230, 255]], [1300, 820, [180, 110, 235, 255]]
].map do |x, y, color|
  shape = RectangleShape.new([120, 120])
  shape.position = [x, y]
  shape.fill_color = color
  shape
end

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

# Two cameras, each sized to its half-window viewport so the aspect is right.
left = View.from_rect(Rect.new(0, 0, HALF, HEIGHT))
left.viewport = [0.0, 0.0, 0.5, 1.0]
right = View.from_rect(Rect.new(0, 0, HALF, HEIGHT))
right.viewport = [0.5, 0.0, 0.5, 1.0]

# The overview shows the whole world, so its centre is fixed and it is zoomed.
right.center = [WORLD_WIDTH / 2, WORLD_HEIGHT / 2]
right.zoom(3.6)

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

  left.center = clamp_center(player.position, left.size)

  window.clear!([16, 18, 28, 255])

  # Draw the same scene once per view.
  [left, right].each do |view|
    window.view = view
    window.draw(grid)
    landmarks.each { |shape| window.draw(shape) }
    window.draw(player)
  end

  # A divider between the two viewports, back in pixel coordinates.
  window.view = window.default_view
  divider = RectangleShape.new([2, HEIGHT])
  divider.position = [HALF - 1, 0]
  divider.fill_color = [235, 235, 245, 255]
  window.draw(divider)
  window.draw(text('left: follow camera', size: 15, position: [12, 8]))
  window.draw(text('right: fixed overview', size: 15, position: [HALF + 12, 8]))
  window.draw(text('arrows move, escape quits', size: 15, position: [12, HEIGHT - 26]))
  window.display!
end

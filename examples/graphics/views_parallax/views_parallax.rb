# frozen_string_literal: true

# Parallax is two cameras over the same world moving at different speeds. The
# near view follows the player; the far view's centre is always half of the
# near one's, so drawn through it the background scrolls at half the speed and
# reads as distant. The arrow keys move, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/views_parallax/views_parallax.rb
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

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML views: parallax')
window.frame_rate = 60

player = CircleShape.new(18)
player.origin = [18, 18]
player.fill_color = [255, 210, 90, 255]
player.position = [WORLD_WIDTH / 2, WORLD_HEIGHT / 2]

# The far layer: a grid and a field of dim "stars", drawn through the slow view.
grid = VertexArray.new
grid.primitive = :lines
(0..WORLD_WIDTH).step(100) do |x|
  grid.append(Vertex.new([x, 0], [40, 46, 66, 255]))
  grid.append(Vertex.new([x, WORLD_HEIGHT], [40, 46, 66, 255]))
end
(0..WORLD_HEIGHT).step(100) do |y|
  grid.append(Vertex.new([0, y], [40, 46, 66, 255]))
  grid.append(Vertex.new([WORLD_WIDTH, y], [40, 46, 66, 255]))
end

stars = VertexArray.new
stars.primitive = :points
srand(7)
300.times do
  stars.append(Vertex.new([rand(WORLD_WIDTH), rand(WORLD_HEIGHT)], [150, 160, 190, 220]))
end

# The near layer: solid landmarks plus the player.
landmarks = [
  [120, 120, [230, 90, 90, 255]], [1400, 200, [90, 200, 130, 255]],
  [260, 800, [90, 170, 230, 255]], [1300, 820, [180, 110, 235, 255]]
].map do |x, y, color|
  shape = RectangleShape.new([120, 120])
  shape.position = [x, y]
  shape.fill_color = color
  shape
end

# Two cameras of the same size; only their centres differ.
near = View.from_rect(Rect.new(0, 0, WIDTH, HEIGHT))
far = View.from_rect(Rect.new(0, 0, WIDTH, HEIGHT))

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

  near.center = clamp_center(player.position, near.size)
  center = near.center
  far.center = [center.x * 0.5, center.y * 0.5]

  window.clear!([12, 14, 24, 255])

  window.view = far
  window.draw(grid)
  window.draw(stars)

  window.view = near
  landmarks.each { |shape| window.draw(shape) }
  window.draw(player)

  window.view = window.default_view
  window.draw(text(
                "near #{near.center.x.to_i},#{near.center.y.to_i}   " \
                "far #{far.center.x.to_i},#{far.center.y.to_i} (half speed)\n" \
                'arrows move, escape quits',
                size: 15
              ))
  window.display!
end

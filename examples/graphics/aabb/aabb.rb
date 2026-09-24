# frozen_string_literal: true

# Axis-aligned bounding boxes (AABBs) and the simplest platformer physics: a
# player box with velocity and gravity, static solid boxes, and collisions
# resolved one axis at a time. `Shape#global_bounds` returns the box's Rect and
# `Rect#intersects?` / `Rect#intersection` give the overlap, so no maths is
# hidden in the binding. Left/right move, space jumps, R resets, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/aabb/aabb.rb
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
GRAVITY = 0.6
JUMP = -12.0
ACCEL = 0.8
MAX_SPEED = 6.0
FRICTION = 0.8
SIZE = 36

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML AABB collisions')
window.frame_rate = 60

player = RectangleShape.new([SIZE, SIZE])
player.origin = [SIZE / 2, SIZE / 2]
player.fill_color = [90, 170, 230, 255]

SOLIDS = [
  [0, HEIGHT - 40, WIDTH, 40], # ground
  [120, HEIGHT - 160, 220, 24], # platform
  [430, HEIGHT - 250, 180, 24], # platform
  [700, HEIGHT - 380, 160, 24]  # platform
].freeze

solids = SOLIDS.map do |x, y, w, h|
  shape = RectangleShape.new([w, h])
  shape.position = [x, y]
  shape.fill_color = [70, 78, 100, 255]
  shape.outline_thickness = 1
  shape.outline_color = [120, 128, 150, 255]
  shape
end

def spawn
  [120.0, 100.0, 0.0, 0.0] # x, y, vx, vy
end

x, y, vx, vy = spawn
on_ground = false
collisions = 0
overlap = nil

# Move the player by one axis at a time and push it back out of any solid it
# overlaps. Resolving axes separately is what keeps corners from sticking.
def move_axis(x, y, vx, vy, axis, solids, size)
  if axis == :x
    x += vx
  else
    y += vy
  end

  box = Rect.new(x - (size / 2), y - (size / 2), size, size)
  hit = nil
  landed = false

  solids.each do |solid|
    solid_box = solid.global_bounds
    next unless box.intersects?(solid_box)

    hit = box.intersection(solid_box)
    if axis == :x
      x = vx.positive? ? solid_box.left - (size / 2) : solid_box.left + solid_box.width + (size / 2)
      vx = 0.0
    else
      landed = true if vy.positive?
      y = vy.positive? ? solid_box.top - (size / 2) : solid_box.top + solid_box.height + (size / 2)
      vy = 0.0
    end
    box = Rect.new(x - (size / 2), y - (size / 2), size, size)
  end

  [x, y, vx, vy, hit, landed]
end

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :Space, :Up then vy = JUMP if on_ground
      when :r
        x, y, vx, vy = spawn
        collisions = 0
      end
    end
  end

  vx += ACCEL if Keyboard.key_pressed?(:Right)
  vx -= ACCEL if Keyboard.key_pressed?(:Left)
  vx = vx.clamp(-MAX_SPEED, MAX_SPEED)
  vx *= FRICTION unless Keyboard.key_pressed?(:Left) || Keyboard.key_pressed?(:Right)

  vy += GRAVITY
  vy = vy.clamp(-40.0, 20.0)

  overlap = nil
  x, _y, vx, _vy, hit, = move_axis(x, y, vx, vy, :x, solids, SIZE)
  overlap = hit if hit
  collisions += 1 if hit

  moved_x, y, _vx, vy, hit, landed = move_axis(x, y, vx, vy, :y, solids, SIZE)
  x = moved_x
  overlap = hit if hit
  collisions += 1 if hit
  on_ground = landed

  player.position = [x, y]

  window.clear!([14, 16, 26, 255])
  solids.each { |shape| window.draw(shape) }

  # Draw the player's AABB and, when they overlap, the intersection rectangle.
  box = player.global_bounds
  marker = RectangleShape.new(box.size)
  marker.position = [box.left, box.top]
  marker.fill_color = [0, 0, 0, 0]
  marker.outline_thickness = 1
  marker.outline_color = [255, 255, 255, 90]
  window.draw(marker)
  window.draw(player)

  if overlap
    patch = RectangleShape.new([overlap.width, overlap.height])
    patch.position = [overlap.left, overlap.top]
    patch.fill_color = [255, 90, 90, 140]
    window.draw(patch)
  end

  window.draw(text(
                "AABB #{SIZE}x#{SIZE}   velocity #{vx.round(1)}, #{vy.round(1)}   " \
                "on_ground: #{on_ground}   resolutions: #{collisions}\n" \
                "overlap: #{overlap ? "#{overlap.width.round}x#{overlap.height.round}" : 'none'}   " \
                'arrows move, space jumps, R resets, escape quits',
                size: 15
              ))
  window.display!
end

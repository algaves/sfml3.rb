# frozen_string_literal: true

# Sprite animation with named clips over a runtime-built atlas. The atlas is
# drawn into an Image pixel by pixel (three rows: idle, walk, run -- four frames
# each), uploaded once with Texture.from_image, and animated by advancing
# Sprite#texture_rect on a Clock. 1/2/3 switch clips, space pauses, +/- change
# speed, F flips horizontally, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/sprite_animation/sprite_animation.rb
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

CELL = 24
COLUMNS = 4
ROWS = 3

SKIN = [240, 205, 170, 255].freeze
SHIRTS = [[90, 170, 230, 255], [110, 200, 120, 255], [235, 140, 90, 255]].freeze
PANTS = [54, 60, 84, 255].freeze

# Draws one 24x24 character frame into +image+ at the (ox, oy) cell origin. The
# pose depends on the row (idle/walk/run) and the frame column.
def draw_character(image, ox, oy, row, column)
  put = lambda do |x, y, color|
    image.set_pixel(ox + x, oy + y, color) if x.between?(0, CELL - 1) && y.between?(0, CELL - 1)
  end
  fill = lambda do |x, y, width, height, color|
    height.times { |dy| width.times { |dx| put.call(x + dx, y + dy, color) } }
  end

  swing = [0, 1, 0, -1][column]
  swing *= 2 if row == 2
  bob = row.zero? ? [0, 0, 1, 0][column] : 0
  shirt = SHIRTS[row]

  fill.call(9, 4 + bob, 6, 6, SKIN)          # head
  fill.call(8, 10 + bob, 8, 7, shirt)        # torso
  fill.call(5, 11 + bob + swing, 3, 2, shirt) # left arm
  fill.call(16, 11 + bob - swing, 3, 2, shirt) # right arm
  fill.call(9, 17 + bob, 3, 6, PANTS)         # left leg
  fill.call(12, 17 + bob, 3, 6, PANTS)        # right leg
  put.call(11, 7 + bob, [30, 32, 44, 255])    # eye
  put.call(13, 7 + bob, [30, 32, 44, 255])
end

def build_atlas
  atlas = Image.from_color([CELL * COLUMNS, CELL * ROWS], Color.new(0, 0, 0, 0))
  ROWS.times do |row|
    COLUMNS.times do |column|
      draw_character(atlas, column * CELL, row * CELL, row, column)
    end
  end
  atlas
end

CLIPS = {
  idle: { row: 0, columns: [0, 1, 2, 3], fps: 4.0 },
  walk: { row: 1, columns: [0, 1, 2, 3], fps: 9.0 },
  run: { row: 2, columns: [0, 1, 2, 3], fps: 15.0 }
}.freeze

WIDTH = 800
HEIGHT = 600

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML sprite animation')
window.frame_rate = 60

atlas_texture = Texture.from_image(build_atlas)
atlas_texture.smooth = false

sprite = Sprite.new(atlas_texture)
sprite.origin = [CELL / 2, CELL / 2]
sprite.position = [WIDTH / 2, 330]
sprite.scale = [8, 8]

clip = :walk
time = 0.0
speed = 1.0
paused = false
flipped = false
clock = Clock.new

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :num1 then clip = :idle
      when :num2 then clip = :walk
      when :num3 then clip = :run
      when :Space then paused = !paused
      when :f
        flipped = !flipped
        sprite.scale = [flipped ? -8 : 8, 8]
        sprite.origin = [CELL / 2, CELL / 2]
      when :Equal, :Add then speed = [speed + 0.25, 4.0].min
      when :Hyphen, :Subtract then speed = [speed - 0.25, 0.25].max
      end
    end
  end

  dt = clock.restart!.as_seconds
  time += dt * speed unless paused

  frames = CLIPS.fetch(clip)
  index = (time * frames[:fps]).to_i % frames[:columns].length
  column = frames[:columns][index]
  sprite.texture_rect = [column * CELL, frames[:row] * CELL, CELL, CELL]

  window.clear!([20, 22, 32, 255])

  # A ground line and a faint cell grid around the sprite.
  ground = RectangleShape.new([WIDTH, 4])
  ground.position = [0, 430]
  ground.fill_color = [44, 50, 70, 255]
  window.draw(ground)
  window.draw(sprite)

  window.draw(text(
                "clip: #{clip}   frame #{index + 1}/#{frames[:columns].length}   " \
                "fps #{frames[:fps]}   speed x#{speed}   #{paused ? 'paused' : 'playing'}   " \
                "facing #{flipped ? 'left' : 'right'}\n" \
                '1 idle, 2 walk, 3 run, space pause, +/- speed, F flip, escape quits',
                size: 16, position: [30, 60]
              ))
  window.display!

end

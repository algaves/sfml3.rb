# frozen_string_literal: true

# Flappy Bird: flap through the gaps in scrolling pipes. Space (or Up, or a
# click) flaps, the bird tilts with its vertical speed, and the ground scrolls
# beneath it from a repeated texture. The best score survives restarts. Escape
# returns to the menu, R restarts.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/flappy_bird/flappy_bird.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

def sound(name, volume: 60)
  buffer = SoundBuffer.from_file(File.join(ASSETS, "#{name}.wav"))
  Sound.new(buffer).tap { |sound| sound.volume = volume }
end

def procedural_texture(width, height)
  pixels = String.new(encoding: Encoding::BINARY)
  height.times do |y|
    width.times do |x|
      color = yield(x, y)
      color += [255] if color.size == 3
      pixels << color.pack('C4')
    end
  end
  Texture.from_image(Image.from_pixels([width, height], pixels))
end

SHEET = Texture.from_file(File.join(ASSETS, 'sprites.png'))
SPRITES = { bird_up: [0, 0], bird_mid: [1, 0], bird_down: [2, 0], pipe: [3, 0] }.freeze
def sprite(name, scale: 1)
  column, row = SPRITES.fetch(name)
  result = Sprite.new(SHEET)
  result.texture_rect = [column * 16, row * 16, 16, 16]
  result.scale = [scale, scale] if scale != 1
  result
end

WIDTH = 480
HEIGHT = 640
GROUND = HEIGHT - 90
GRAVITY = 0.5
FLAP = -8.4
PIPE_SPEED = 2.6
PIPE_GAP = 165
PIPE_WIDTH = 64
PIPE_SPACING = 240

def overlap?(a, b)
  a[0] < b[0] + b[2] && a[0] + a[2] > b[0] && a[1] < b[1] + b[3] && a[1] + a[3] > b[1]
end

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML flappy bird')
window.frame_rate = 60
flap_sound = sound('jump', volume: 45)
score_sound = sound('beep', volume: 50)
hit_sound = sound('explode', volume: 55)

bird = sprite(:bird_mid, scale: 3)
bird.origin = [8, 8]
BIRD_FRAMES = %i[bird_up bird_mid bird_down bird_mid].freeze

ground_texture = procedural_texture(32, 32) do |x, y|
  ((x / 8) + (y / 8)).even? ? [72, 138, 82, 255] : [58, 118, 68, 255]
end
ground_texture.repeated = true
ground = RectangleShape.new([WIDTH, 90])
ground.position = [0, GROUND]
ground.texture = ground_texture
ground.texture_rect = [0, 0, WIDTH, 90]

backdrop = procedural_texture(4, 160) do |_x, y|
  t = y / 160.0
  [(38 + (t * 60)).to_i, (120 + (t * 90)).to_i, (170 + (t * 60)).to_i, 255]
end
sky = RectangleShape.new([WIDTH, GROUND])
sky.texture = backdrop
sky.texture_rect = [0, 0, WIDTH, GROUND]

hud = text('', size: 22, position: [16, 16])
small = text('', size: 16, position: [16, HEIGHT - 34])
animation = Clock.new
ground_offset = 0.0

state = :playing
bird_y = 0.0
bird_vy = 0.0
bird_angle = 0.0
pipes = []
score = 0
best = 0
frame_index = 0

start = lambda do
  bird_y = HEIGHT * 0.4
  bird_vy = 0.0
  bird_angle = 0.0
  pipes = [{ x: WIDTH + 160.0, gap: rand(120..(GROUND - 200)), scored: false }]
  score = 0
  state = :playing
end

flap = lambda do
  bird_vy = FLAP
  bird_angle = -28
  flap_sound.play! if state == :playing
end

start.call

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      if key == :escape
        window.close!
      elsif state == :playing && %i[Space Up].include?(key)
        flap.call
      elsif state == :game_over && key == :r
        start.call
      end
    when 'mouse-button-pressed'
      state == :playing ? flap.call : nil
    end
  end

  if state == :playing
    bird_vy = [bird_vy + GRAVITY, 14.0].min
    bird_y += bird_vy
    bird_angle = [bird_angle + 3, 90].min
    ground_offset = (ground_offset + PIPE_SPEED) % 32
    ground.texture_rect = [ground_offset, 0, WIDTH, 90]

    pipes.each { |pipe| pipe[:x] -= PIPE_SPEED }
    if pipes.last[:x] < WIDTH - PIPE_SPACING
      pipes << { x: pipes.last[:x] + PIPE_SPACING, gap: rand(120..(GROUND - 200)), scored: false }
    end

    pipes.each do |pipe|
      next if pipe[:scored] || pipe[:x] + PIPE_WIDTH < bird.position.x

      pipe[:scored] = true
      score += 1
      score_sound.play!
    end
    pipes.reject! { |pipe| pipe[:x] < -PIPE_WIDTH }

    bird_rect = [bird.position.x - 24, bird_y - 24, 48, 48]
    hit = bird_y < 20 || bird_y > GROUND - 10 ||
          pipes.any? do |pipe|
            top = [pipe[:x], 0, PIPE_WIDTH, pipe[:gap]]
            bottom = [pipe[:x], pipe[:gap] + PIPE_GAP, PIPE_WIDTH, GROUND - pipe[:gap] - PIPE_GAP]
            overlap?(bird_rect, top) || overlap?(bird_rect, bottom)
          end
    if hit
      hit_sound.play!
      best = [best, score].max
      state = :game_over
    end
  end

  bird.position = [WIDTH * 0.28, bird_y]
  bird.rotation = bird_angle

  if animation.elapsed_time.as_seconds > 0.12
    animation.restart!
    frame_index = (frame_index + 1) % BIRD_FRAMES.length
  end
  column, row = SPRITES.fetch(BIRD_FRAMES[frame_index])
  bird.texture_rect = [column * 16, row * 16, 16, 16]

  window.clear!([110, 180, 220, 255])
  window.draw(sky)
  pipes.each do |pipe|
    top = sprite(:pipe, scale: 4)
    top.origin = [8, 0]
    top.position = [pipe[:x] + (PIPE_WIDTH / 2), 0]
    top.scale = [4, pipe[:gap] / 16.0]
    top.color = Color::WHITE
    window.draw(top)

    bottom = sprite(:pipe, scale: 4)
    bottom.origin = [8, 16]
    bottom.position = [pipe[:x] + (PIPE_WIDTH / 2), GROUND]
    bottom.scale = [4, (GROUND - pipe[:gap] - PIPE_GAP) / 16.0]
    window.draw(bottom)
  end
  window.draw(ground)
  window.draw(bird)

  hud.string = score.to_s
  small.string = "best #{best}   escape quits"
  window.draw(hud)
  window.draw(small)

  if state == :game_over
    window.draw(text("Game over\nscore #{score}  best #{best}\n\nR to play again, escape quits",
                     size: 24, position: [80, 220]))
  end

  window.display!
end

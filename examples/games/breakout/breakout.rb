# frozen_string_literal: true

# Breakout: a paddle, a ball and five rows of bricks. The mouse (or the arrow
# keys) moves the paddle; the ball speeds up as the board empties. Losing the
# ball costs a life, clearing every brick wins, R restarts and escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/breakout/breakout.rb
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

WIDTH = 640
HEIGHT = 480
ROWS = 5
COLUMNS = 10
BRICK = [56, 20].freeze
GAP = [4, 4].freeze
MARGIN = [(WIDTH - ((COLUMNS * BRICK[0]) + ((COLUMNS - 1) * GAP[0]))) / 2, 56].freeze
BALL_RADIUS = 8

Brick = Struct.new(:x, :y, :width, :height, :alive)

def build_bricks
  ROWS.times.flat_map do |row|
    COLUMNS.times.map do |column|
      Brick.new(MARGIN[0] + (column * (BRICK[0] + GAP[0])),
                MARGIN[1] + (row * (BRICK[1] + GAP[1])),
                BRICK[0], BRICK[1], true)
    end
  end
end

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML breakout')
window.frame_rate = 60
hud = text('', size: 16, position: [10, 8])
bounce_sound = sound('bounce')
break_sound = sound('shoot')
lose_sound = sound('explode')

game = {}

reset = lambda do
  game[:paddle] = RectangleShape.new([92, 14])
  game[:paddle].origin = [46, 7]
  game[:paddle].position = [WIDTH / 2, HEIGHT - 40]
  game[:ball] = { x: WIDTH / 2.0, y: HEIGHT - 60.0, vx: 3.6, vy: -3.6 }
  game[:bricks] = build_bricks
  game[:lives] = 3
  game[:score] = 0
  game[:state] = :serve
end
reset.call

while window.open?
  restart = false
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      window.close! if key == :escape
      restart = true if key == :r
      game[:state] = :playing if key == :Space && game[:state] == :serve
    end
  end
  reset.call if restart

  paddle = game[:paddle]
  ball = game[:ball]

  if game[:state] == :serve
    ball[:x] = paddle.position.x
    ball[:y] = paddle.position.y - 20
  elsif game[:state] == :playing
    ball[:x] += ball[:vx]
    ball[:y] += ball[:vy]

    if (ball[:x] - BALL_RADIUS).negative?
      ball[:x] = BALL_RADIUS
      ball[:vx] = ball[:vx].abs
      bounce_sound.play!
    elsif ball[:x] + BALL_RADIUS > WIDTH
      ball[:x] = WIDTH - BALL_RADIUS
      ball[:vx] = -ball[:vx].abs
      bounce_sound.play!
    end
    if (ball[:y] - BALL_RADIUS).negative?
      ball[:y] = BALL_RADIUS
      ball[:vy] = ball[:vy].abs
      bounce_sound.play!
    end

    # Paddle: only bounce while the ball is heading down, using the hit offset
    # for a little English.
    if ball[:vy].positive? &&
       ball[:y] + BALL_RADIUS >= paddle.position.y - 7 &&
       ball[:y] - BALL_RADIUS <= paddle.position.y + 7 &&
       (ball[:x] - paddle.position.x).abs <= 46 + BALL_RADIUS
      offset = ((ball[:x] - paddle.position.x) / 46.0).clamp(-1.0, 1.0)
      ball[:vy] = -ball[:vy].abs
      ball[:vx] = (offset * 5.0) + (ball[:vx].negative? ? -0.5 : 0.5)
      ball[:x] += ball[:vx]
      bounce_sound.play!
    end

    # Bricks: AABB overlap, reflect on the shallower axis.
    game[:bricks].each do |brick|
      next unless brick.alive

      overlap_x = (BRICK[0] / 2.0) + BALL_RADIUS - (ball[:x] - (brick.x + (BRICK[0] / 2.0))).abs
      overlap_y = (BRICK[1] / 2.0) + BALL_RADIUS - (ball[:y] - (brick.y + (BRICK[1] / 2.0))).abs
      next unless overlap_x.positive? && overlap_y.positive?

      brick.alive = false
      game[:score] += 10
      break_sound.play!
      if overlap_x < overlap_y
        ball[:vx] = -ball[:vx]
        ball[:x] += ball[:vx]
      else
        ball[:vy] = -ball[:vy]
        ball[:y] += ball[:vy]
      end
      break
    end

    if ball[:y] - BALL_RADIUS > HEIGHT
      game[:lives] -= 1
      lose_sound.play!
      game[:state] = game[:lives].zero? ? :game_over : :serve
    end
  end

  paddle_x = Mouse.position(window).x
  paddle_x = paddle.position.x - 7 if Keyboard.key_pressed?(:Left)
  paddle_x = paddle.position.x + 7 if Keyboard.key_pressed?(:Right)
  paddle.position = [paddle_x.clamp(46, WIDTH - 46), HEIGHT - 40]

  window.clear!([18, 22, 30, 255])

  game[:bricks].each do |brick|
    next unless brick.alive

    shape = RectangleShape.new([brick.width, brick.height])
    shape.position = [brick.x, brick.y]
    row = ((brick.y - MARGIN[1]) / (BRICK[1] + GAP[1])).to_i
    shape.fill_color = [40 + (row * 35), 120, 220 - (row * 30), 255]
    window.draw(shape)
  end

  orb = CircleShape.new(BALL_RADIUS)
  orb.origin = [BALL_RADIUS, BALL_RADIUS]
  orb.position = [ball[:x], ball[:y]]
  orb.fill_color = [235, 235, 245, 255]
  window.draw(orb)
  window.draw(paddle)

  remaining = game[:bricks].count(&:alive)
  game[:state] = :won if remaining.zero?
  status = case game[:state]
           when :serve then 'click space to serve'
           when :game_over then 'GAME OVER -- R restarts'
           when :won then 'CLEARED! R restarts'
           else 'mouse or arrows move the paddle'
           end
  hud.string = "score #{game[:score]}   lives #{game[:lives]}   bricks #{remaining}   #{status}"
  window.draw(hud)
  window.display!

end

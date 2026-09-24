# frozen_string_literal: true

# Asteroids: thrust, rotate and shoot, with everything wrapping around the
# edges. Asteroids split into two smaller ones when hit; the ship dies on
# contact and respawns with a short window of invulnerability. Left/Right
# rotate, Up thrusts, Space fires, R restarts, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/asteroids/asteroids.rb
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

WIDTH = 760
HEIGHT = 560
RADII = { large: 46, medium: 26, small: 14 }.freeze
SCORES = { large: 20, medium: 50, small: 100 }.freeze

def wrap(point)
  [point[0] % WIDTH, point[1] % HEIGHT]
end

def distance(a, b)
  Math.sqrt(((a[0] - b[0])**2) + ((a[1] - b[1])**2))
end

def asteroid_points(radius)
  count = 9
  count.times.map do |index|
    angle = (index.to_f / count) * Math::PI * 2
    length = radius * (0.75 + (rand * 0.35))
    [Math.cos(angle) * length, Math.sin(angle) * length]
  end
end

def make_asteroid(size, x, y, vx, vy)
  radius = RADII[size]
  { size: size, x: x, y: y, vx: vx, vy: vy, radius: radius, points: asteroid_points(radius),
    rotation: rand(360.0), spin: rand(-1.0..1.0) }
end

def spawn_field
  Array.new(4) do
    make_asteroid(:large, rand(WIDTH), rand(HEIGHT), rand(-1.4..1.4), rand(-1.4..1.4))
  end
end

def split(game, rock)
  smaller = rock[:size] == :large ? :medium : :small
  2.times do
    angle = rand * Math::PI * 2
    speed = rand(0.6..1.8)
    game[:asteroids] << make_asteroid(smaller, rock[:x], rock[:y],
                                      Math.cos(angle) * speed, Math.sin(angle) * speed)
  end
end

def handle_hits(game, boom_sound)
  game[:bullets].reject! do |bullet|
    index = game[:asteroids].index do |rock|
      distance([bullet[:x], bullet[:y]], [rock[:x], rock[:y]]) < rock[:radius]
    end
    next false unless index

    rock = game[:asteroids].delete_at(index)
    game[:score] += SCORES[rock[:size]]
    boom_sound.play!
    split(game, rock) unless rock[:size] == :small
    true
  end
end

def ship_hit?(ship, asteroids)
  asteroids.any? { |rock| distance([ship[:x], ship[:y]], [rock[:x], rock[:y]]) < rock[:radius] + 8 }
end

def draw_asteroid(window, rock)
  shape = ConvexShape.new(rock[:points].length)
  rock[:points].each_with_index { |point, index| shape.set_point(index, point) }
  shape.position = [rock[:x], rock[:y]]
  shape.rotation = rock[:rotation]
  shape.fill_color = [26, 30, 44, 255]
  shape.outline_thickness = 2
  shape.outline_color = [170, 180, 200, 255]
  window.draw(shape)
end

def draw_bullet(window, bullet)
  orb = CircleShape.new(2.5)
  orb.origin = [2.5, 2.5]
  orb.position = [bullet[:x], bullet[:y]]
  orb.fill_color = [255, 230, 140, 255]
  window.draw(orb)
end

def draw_ship(window, ship)
  shape = ConvexShape.new(3)
  shape.set_point(0, [14, 0])
  shape.set_point(1, [-10, 9])
  shape.set_point(2, [-10, -9])
  shape.position = [ship[:x], ship[:y]]
  shape.rotation = ship[:angle]
  shape.fill_color = [120, 220, 255, 255]
  window.draw(shape)
end

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML asteroids')
window.frame_rate = 60
hud = text('', size: 16, position: [10, 8])
shoot_sound = sound('shoot')
boom_sound = sound('explode')
thump_sound = sound('bounce')

game = {}
frame = 0

reset = lambda do
  game[:ship] = { x: WIDTH / 2.0, y: HEIGHT / 2.0, vx: 0.0, vy: 0.0, angle: 0.0, invulnerable: 120 }
  game[:bullets] = []
  game[:asteroids] = spawn_field
  game[:lives] = 3
  game[:score] = 0
  game[:state] = :playing
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
      if key == :Space && game[:state] == :playing
        ship = game[:ship]
        radians = ship[:angle] * Math::PI / 180
        game[:bullets] << { x: ship[:x], y: ship[:y], vx: (Math.cos(radians) * 9) + ship[:vx],
                            vy: (Math.sin(radians) * 9) + ship[:vy], life: 70 }
        shoot_sound.play!
      end
    end
  end
  reset.call if restart

  ship = game[:ship]

  if game[:state] == :playing
    ship[:angle] -= 4.5 if Keyboard.key_pressed?(:Left)
    ship[:angle] += 4.5 if Keyboard.key_pressed?(:Right)
    if Keyboard.key_pressed?(:Up)
      radians = ship[:angle] * Math::PI / 180
      ship[:vx] += Math.cos(radians) * 0.16
      ship[:vy] += Math.sin(radians) * 0.16
      thump_sound.play! if (frame % 8).zero?
    end
    ship[:vx] *= 0.992
    ship[:vy] *= 0.992
    ship[:x], ship[:y] = wrap([ship[:x] + ship[:vx], ship[:y] + ship[:vy]])
    ship[:invulnerable] -= 1

    game[:bullets].each do |bullet|
      bullet[:x], bullet[:y] = wrap([bullet[:x] + bullet[:vx], bullet[:y] + bullet[:vy]])
      bullet[:life] -= 1
    end
    game[:bullets].reject! { |bullet| bullet[:life] <= 0 }

    game[:asteroids].each do |rock|
      rock[:x], rock[:y] = wrap([rock[:x] + rock[:vx], rock[:y] + rock[:vy]])
      rock[:rotation] += rock[:spin]
    end

    handle_hits(game, boom_sound)
    if game[:asteroids].empty?
      game[:state] = :won
    elsif ship_hit?(ship, game[:asteroids]) && ship[:invulnerable] <= 0
      boom_sound.play!
      game[:lives] -= 1
      if game[:lives].zero?
        game[:state] = :game_over
      else
        game[:ship] = { x: WIDTH / 2.0, y: HEIGHT / 2.0, vx: 0.0, vy: 0.0, angle: 0.0,
                        invulnerable: 120 }
        ship = game[:ship]
      end
    end
  end

  window.clear!([12, 14, 22, 255])
  game[:asteroids].each { |rock| draw_asteroid(window, rock) }
  game[:bullets].each { |bullet| draw_bullet(window, bullet) }
  draw_ship(window, ship) if game[:state] == :playing && (!ship[:invulnerable].positive? || (frame % 10 < 5))
  hud.string = "score #{game[:score]}   lives #{game[:lives]}   rocks #{game[:asteroids].length}   " \
               "#{game[:state] == :playing ? 'arrows thrust/rotate, space fires' : game[:state].to_s.upcase}"
  window.draw(hud)
  window.display!

  frame += 1
end

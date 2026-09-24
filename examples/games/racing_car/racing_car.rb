# frozen_string_literal: true

# Top-down racing on an oval. The menu picks one player (against a waypoint AI)
# or two (arrows vs WASD). Accelerate with Up/W, brake with Down/S, steer with
# Left/Right; leaving the asphalt slows the car down. Cross the checkpoints in
# order to score a lap; the best lap is kept. Escape returns to the menu.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/racing_car/racing_car.rb
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
SPRITES = { car: [6, 0] }.freeze
def sprite(name, scale: 1)
  column, row = SPRITES.fetch(name)
  result = Sprite.new(SHEET)
  result.texture_rect = [column * 16, row * 16, 16, 16]
  result.scale = [scale, scale] if scale != 1
  result
end

WIDTH = 880
HEIGHT = 620
CX = 900
CY = 650
OUTER = [820, 520].freeze
INNER = [560, 340].freeze
MID = [690, 430].freeze
CHECKPOINTS = 10
LAPS = 3
MAX_SPEED = 7.2
ACCEL = 0.14
BRAKE = 0.16
TURN = 3.4

def ellipse_shape(center, radii, color, points = 56)
  shape = ConvexShape.new(points)
  points.times do |index|
    angle = (index.to_f / points) * Math::PI * 2
    shape.set_point(index, [Math.cos(angle) * radii[0], Math.sin(angle) * radii[1]])
  end
  shape.position = center
  shape.fill_color = color
  shape
end

def on_track?(position)
  dx = (position[0] - CX) / OUTER[0].to_f
  dy = (position[1] - CY) / OUTER[1].to_f
  outer = (dx * dx) + (dy * dy)
  ix = (position[0] - CX) / INNER[0].to_f
  iy = (position[1] - CY) / INNER[1].to_f
  inner = (ix * ix) + (iy * iy)
  1.0.between?(outer, inner)
end

def checkpoint_position(index)
  angle = (index.to_f / CHECKPOINTS) * Math::PI * 2
  [CX + (Math.cos(angle) * MID[0]), CY + (Math.sin(angle) * MID[1])]
end

Car = Struct.new(:x, :y, :angle, :speed, :lap, :checkpoint, :laps, :lap_start, :best, :ai)

def new_car(lane, ai)
  start = checkpoint_position(0)
  Car.new(start[0], start[1] + (lane * 26), 90.0, 0.0, 0, 0, 0, 0.0, nil, ai)
end

def steer_car(car, throttle, turn, delta)
  car.speed += throttle * ACCEL
  car.speed -= BRAKE if throttle.negative?
  car.speed = car.speed.clamp(-MAX_SPEED * 0.4, MAX_SPEED)
  on_asphalt = on_track?([car.x, car.y])
  car.speed *= on_asphalt ? 0.992 : 0.90
  car.angle += turn * TURN * (car.speed.abs / MAX_SPEED) * delta * 60
  radians = car.angle * Math::PI / 180
  car.x += Math.cos(radians) * car.speed
  car.y += Math.sin(radians) * car.speed
end

def advance_checkpoints(car)
  target = checkpoint_position(car.checkpoint)
  return unless Math.hypot(car.x - target[0], car.y - target[1]) < 78

  car.checkpoint += 1
  return unless car.checkpoint >= CHECKPOINTS

  car.checkpoint = 0
  elapsed = car.lap_start
  car.best = elapsed if car.best.nil? || elapsed < car.best
  car.laps += 1
  car.lap_start = 0.0
end

def ai_controls(car)
  target = checkpoint_position(car.checkpoint)
  desired = Math.atan2(target[1] - car.y, target[0] - car.x) * 180 / Math::PI
  difference = ((desired - car.angle + 540) % 360) - 180
  turn = difference.clamp(-1.0, 1.0) * 0.9
  throttle = difference.abs > 55 ? 0.2 : 1.0
  [throttle, turn]
end

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML racing')
window.frame_rate = 60
clock = Clock.new
camera = View.new
camera.size = [WIDTH, HEIGHT]
hud = text('', size: 16, position: [12, 10])
big = text('', size: 30, position: [80, 90])

grass = procedural_texture(32, 32) do |x, y|
  ((x / 8) + (y / 8)).even? ? [58, 110, 62, 255] : [50, 98, 56, 255]
end
grass.repeated = true

cars = nil

# Two players by default (arrows vs WASD); M swaps the second for the AI.
start_race = lambda do |ai|
  cars = [new_car(-1, false), new_car(1, ai)]
  clock.restart!
end

start_race.call(false)

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :m then start_race.call(!cars[1].ai)
      when :r then start_race.call(cars[1].ai)
      end
    end
  end

  delta = clock.restart!.as_seconds
  cars.each { |car| car.lap_start += delta }

  cars.each_with_index do |car, index|
    if car.ai
      throttle, turn = ai_controls(car)
    elsif index.zero?
      throttle = 0.0
      throttle = 1.0 if Keyboard.key_pressed?(:Up)
      throttle = -1.0 if Keyboard.key_pressed?(:Down)
      turn = 0.0
      turn -= 1.0 if Keyboard.key_pressed?(:Left)
      turn += 1.0 if Keyboard.key_pressed?(:Right)
    else
      throttle = 0.0
      throttle = 1.0 if Keyboard.key_pressed?(:w)
      throttle = -1.0 if Keyboard.key_pressed?(:s)
      turn = 0.0
      turn -= 1.0 if Keyboard.key_pressed?(:a)
      turn += 1.0 if Keyboard.key_pressed?(:d)
    end
    steer_car(car, throttle, turn, delta)
    advance_checkpoints(car)
  end

  leader = cars.max_by(&:laps)
  camera.center = [leader.x, leader.y]

  window.view = camera
  window.clear!([44, 90, 52, 255])

  background = RectangleShape.new([4000, 3000])
  background.origin = [2000, 1500]
  background.position = [CX, CY]
  background.texture = grass
  background.texture_rect = [0, 0, 4000, 3000]
  window.draw(background)

  window.draw(ellipse_shape([CX, CY], OUTER, [58, 60, 68, 255]))
  window.draw(ellipse_shape([CX, CY], [OUTER[0] - 6, OUTER[1] - 6], [74, 76, 86, 255]))
  window.draw(ellipse_shape([CX, CY], INNER, [54, 104, 58, 255]))

  finish = checkpoint_position(0)
  start_line = RectangleShape.new([26, 150])
  start_line.origin = [13, 75]
  start_line.position = finish
  start_line.rotation = 90
  start_line.fill_color = [230, 230, 235, 255]
  window.draw(start_line)

  cars&.each_with_index do |car, index|
    car_sprite = sprite(:car, scale: 2)
    car_sprite.origin = [8, 8]
    car_sprite.position = [car.x, car.y]
    car_sprite.rotation = car.angle
    car_sprite.color = index.zero? ? Color.new(150, 220, 255, 255) : Color.new(255, 200, 150, 255)
    window.draw(car_sprite)
  end

  window.view = window.default_view

  lines = cars.each_with_index.map do |car, index|
    best = car.best ? format('%.2fs', car.best) : '-'
    "P#{index + 1}#{' (cpu)' if car.ai}  lap #{[car.laps + 1, LAPS].min}/#{LAPS}  " \
      "speed #{(car.speed.abs * 30).round}  best #{best}"
  end
  lines << "#{cars[1].ai ? '1 player' : '2 players'} -- M changes mode, R restarts, escape quits"
  hud.string = lines.join("\n")
  window.draw(hud)

  winner = cars.find { |car| car.laps >= LAPS }
  if winner
    big.string = "P#{cars.index(winner) + 1} wins!  R to race again, escape quits"
    window.draw(big)
  end

  window.display!
end

# frozen_string_literal: true

# Doodle Jump: the doodler bounces off platforms forever and the camera chases
# it upwards. Platforms are normal, moving, or breakable, and some carry a
# spring that launches higher. Height is the score; falling off the bottom ends
# the run. Left/Right (or A/D) steer, the best score survives restarts, escape
# returns to the menu.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/doodle_jump.rb
# (headless: SFML_EXAMPLE_FRAMES=600 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

WIDTH = 420
HEIGHT = 720
GRAVITY = 0.42
BOUNCE = 12.5
SPRING_BOUNCE = 20.0
MOVE = 5.5
PLATFORM_W = 60

def overlap?(a, b)
  a[0] < b[0] + b[2] && a[0] + a[2] > b[0] && a[1] < b[1] + b[3] && a[1] + a[3] > b[1]
end

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML doodle jump')
window.frame_rate = 60
bounce_sound = ExampleSupport.sound('jump', volume: 45)
spring_sound = ExampleSupport.sound('shoot', volume: 45)
fall_sound = ExampleSupport.sound('explode', volume: 50)

doodler = ExampleSupport.sprite(:doodler, scale: 3)
doodler.origin = [8, 8]
camera = View.new
camera.size = [WIDTH, HEIGHT]
hud = ExampleSupport.text('', size: 20, position: [12, 12])

state = :playing
frame = 0
best = 0
score = 0
dx = 0.0
dy = 0.0
facing = 1
platforms = []
top_y = 0.0
cam_y = 0.0

spawn_platform = lambda do |y|
  kind = if y.negative? && rand < 0.25 then (rand < 0.5 ? :moving : :breakable) else :normal end
  platforms << { x: rand(10..(WIDTH - PLATFORM_W - 10)), y: y, kind: kind, vx: [-1.4, 1.4].sample,
                 spring: kind == :normal && rand < 0.12, gone: false }
end

start = lambda do
  dx = 0.0
  dy = -BOUNCE * 0.6
  score = 0
  facing = 1
  doodler.position = [WIDTH / 2.0, 40.0]
  platforms = []
  platforms << { x: (WIDTH - PLATFORM_W) / 2.0, y: 90.0, kind: :normal, vx: 0, spring: false, gone: false }
  top_y = 90.0
  6.times do
    top_y -= rand(70..110)
    spawn_platform.call(top_y)
  end
  cam_y = 40 - (HEIGHT * 0.35)
  camera.center = [WIDTH / 2, cam_y]
  state = :playing
end

start.call

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      if key == :escape
        window.close!
      elsif state == :game_over && key == :r
        start.call
      end
    end
  end

  if state == :playing
    dx = -MOVE if Keyboard.key_pressed?(:Left) || Keyboard.key_pressed?(:a)
    dx = MOVE if Keyboard.key_pressed?(:Right) || Keyboard.key_pressed?(:d)
    dx *= 0.9 unless Keyboard.key_pressed?(:Left) || Keyboard.key_pressed?(:a) ||
                     Keyboard.key_pressed?(:Right) || Keyboard.key_pressed?(:d)
    facing = -1 if dx.negative?
    facing = 1 if dx.positive?

    dy = [dy + GRAVITY, 16.0].min
    doodler.position = [doodler.position.x + dx, doodler.position.y + dy]
    doodler.position = [-20.0, doodler.position.y] if doodler.position.x < -20
    doodler.position = [WIDTH + 20.0, doodler.position.y] if doodler.position.x > WIDTH + 20

    feet = doodler.position.y + 16
    platforms.each do |platform|
      next if platform[:gone]

      platform[:x] += platform[:vx] if platform[:kind] == :moving
      platform[:vx] = -platform[:vx] if platform[:x] < 4 || platform[:x] > WIDTH - PLATFORM_W - 4

      doodler_rect = [doodler.position.x - 14, doodler.position.y - 14, 28, 28]
      next unless dy.positive? && overlap?(doodler_rect, [platform[:x], platform[:y], PLATFORM_W, 14]) &&
                  feet <= platform[:y] + 18

      if platform[:spring]
        dy = -SPRING_BOUNCE
        spring_sound.play!
      else
        dy = -BOUNCE
        bounce_sound.play!
      end
      platform[:gone] = true if platform[:kind] == :breakable
      break
    end

    climbed = ((90 - doodler.position.y) / 10).to_i
    score = climbed if climbed > score

    needed = camera.center.y - (HEIGHT / 2) - 120
    while top_y > needed
      top_y -= rand(70..110)
      spawn_platform.call(top_y)
    end

    cam_y = [cam_y, doodler.position.y - (HEIGHT * 0.35)].min
    camera.center = [WIDTH / 2, cam_y]

    platforms.reject! { |platform| platform[:y] > camera.center.y + (HEIGHT / 2) - 40 }

    if doodler.position.y > camera.center.y + (HEIGHT / 2) + 40
      fall_sound.play!
      best = [best, score].max
      state = :game_over
    end
  end

  doodler.scale = [3 * facing, 3]

  window.view = camera
  window.clear!([36, 46, 70, 255])

  platforms.each do |platform|
    next if platform[:gone]

    tile = ExampleSupport.sprite(:platform, scale: 1)
    tile.origin = [0, 0]
    tile.position = [platform[:x], platform[:y]]
    tile.scale = [PLATFORM_W / 16.0, 1]
    tile.color = platform[:kind] == :breakable ? Color.new(190, 140, 90, 255) : Color::WHITE
    window.draw(tile)

    next unless platform[:spring]

    spring = ExampleSupport.sprite(:spring, scale: 1)
    spring.position = [platform[:x] + ((PLATFORM_W - 16) / 2), platform[:y] - 14]
    window.draw(spring)
  end

  window.draw(doodler)
  window.view = window.default_view

  hud.string = "#{score}   best #{best}"
  window.draw(hud)

  if state == :game_over
    window.draw(ExampleSupport.text("Fell!\nscore #{score}  best #{best}\n\nR to retry, escape quits",
                                    size: 24, position: [70, 280]))
  end

  window.display!
  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

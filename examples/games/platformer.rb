# frozen_string_literal: true

# Platformer: AABB tile collision, gravity, jumping and a scrolling camera.
# Collect every coin and reach the flag on the right; falling off the level
# restarts the run. Left/Right (or A/D) move, Up or Space jumps, R restarts,
# escape quits. The camera is a View clamped to the level bounds, and the HUD is
# drawn through the window's default view so it stays put.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/platformer.rb
# (headless: SFML_EXAMPLE_FRAMES=600 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

SCREEN = [800, 480].freeze
LEVEL_WIDTH = 2400
GRAVITY = 0.7
JUMP = -12.5
RUN = 0.9
MAX_RUN = 6.0

PLATFORMS = [
  [0, 420, LEVEL_WIDTH, 60],
  [220, 330, 140, 20],
  [430, 270, 120, 20],
  [640, 350, 150, 20],
  [870, 290, 120, 20],
  [1050, 230, 140, 20],
  [1270, 330, 170, 20],
  [1510, 270, 120, 20],
  [1700, 350, 160, 20],
  [1950, 300, 200, 20]
].freeze

COINS = [
  [280, 295], [350, 295], [480, 235], [700, 315], [900, 255], [1110, 195],
  [1330, 295], [1560, 235], [1760, 315], [2050, 265]
].freeze

GOAL = [LEVEL_WIDTH - 90, 360, 30, 60].freeze

def overlap?(a, b)
  a[0] < b[0] + b[2] && a[0] + a[2] > b[0] && a[1] < b[1] + b[3] && a[1] + a[3] > b[1]
end

def move_player(player)
  player[:x] += player[:vx]
  PLATFORMS.each do |platform|
    next unless overlap?([player[:x], player[:y], player[:w], player[:h]], platform)

    player[:x] = player[:vx].positive? ? platform[0] - player[:w] : platform[0] + platform[2]
    player[:vx] = 0.0
  end

  player[:grounded] = false
  player[:y] += player[:vy]
  PLATFORMS.each do |platform|
    next unless overlap?([player[:x], player[:y], player[:w], player[:h]], platform)

    if player[:vy].positive?
      player[:y] = platform[1] - player[:h]
      player[:grounded] = true
    else
      player[:y] = platform[1] + platform[3]
    end
    player[:vy] = 0.0
  end
end

def draw_level(window)
  PLATFORMS.each do |platform|
    shape = RectangleShape.new([platform[2], platform[3]])
    shape.position = [platform[0], platform[1]]
    shape.fill_color = platform[3] > 40 ? [66, 92, 58, 255] : [128, 98, 70, 255]
    window.draw(shape)
  end

  flag = RectangleShape.new([GOAL[2], GOAL[3]])
  flag.position = [GOAL[0], GOAL[1]]
  flag.fill_color = [235, 200, 90, 255]
  window.draw(flag)
end

def draw_coin(window, coin)
  shape = CircleShape.new(9)
  shape.origin = [9, 9]
  shape.position = [coin[:x], coin[:y]]
  shape.fill_color = [235, 200, 90, 255]
  shape.outline_thickness = 2
  shape.outline_color = [160, 120, 30, 255]
  window.draw(shape)
end

def draw_player(window, player)
  shape = RectangleShape.new([player[:w], player[:h]])
  shape.position = [player[:x], player[:y]]
  shape.fill_color = player[:grounded] ? [120, 220, 255, 255] : [90, 170, 230, 255]
  window.draw(shape)
end

window = Window.new(VideoMode.new(SCREEN[0], SCREEN[1], 32), 'SFML platformer')
window.frame_rate = 60
event = Event.new
hud = ExampleSupport.text('', size: 16, position: [10, 8])
coin_sound = ExampleSupport.sound('beep')
jump_sound = ExampleSupport.sound('jump')
win_sound = ExampleSupport.sound('eat')
camera = View.new
camera.size = SCREEN
frame = 0

game = {}
reset = lambda do
  game[:player] = { x: 60.0, y: 320.0, w: 28.0, h: 36.0, vx: 0.0, vy: 0.0, grounded: false }
  game[:coins] = COINS.map { |x, y| { x: x, y: y, taken: false } }
  game[:score] = 0
  game[:state] = :playing
end
reset.call

loop do
  restart = false
  jump = false
  while window.poll_event!(event)
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.key[:code]
      window.close! if key == :escape
      restart = true if key == :r
      jump = true if %i[Up Space].include?(key)
    end
  end
  reset.call if restart

  player = game[:player]

  if game[:state] == :playing
    player[:vx] += RUN if Keyboard.pressed?(:Left) || Keyboard.pressed?(:a)
    player[:vx] -= RUN if Keyboard.pressed?(:Right) || Keyboard.pressed?(:d)
    player[:vx] = player[:vx].clamp(-MAX_RUN, MAX_RUN)
    player[:vx] *= player[:grounded] ? 0.82 : 0.94
    player[:vy] = [player[:vy] + GRAVITY, 18.0].min

    if jump && player[:grounded]
      player[:vy] = JUMP
      player[:grounded] = false
      jump_sound.play
    end

    move_player(player)

    if player[:y] > SCREEN[1] + 100
      reset.call
      player = game[:player]
    elsif player[:x] + player[:w] >= GOAL[0] && game[:coins].all? { |coin| coin[:taken] }
      game[:state] = :won
      win_sound.play
    end

    player[:x] = player[:x].clamp(0, LEVEL_WIDTH - player[:w])
  end

  game[:coins].each do |coin|
    next if coin[:taken]
    next unless (coin[:x] - (player[:x] + (player[:w] / 2))).abs < 24 &&
                (coin[:y] - (player[:y] + (player[:h] / 2))).abs < 24

    coin[:taken] = true
    game[:score] += 1
    coin_sound.play
  end

  camera.center = [(player[:x] + (player[:w] / 2)).clamp(SCREEN[0] / 2, LEVEL_WIDTH - (SCREEN[0] / 2)),
                   SCREEN[1] / 2]
  window.view = camera

  window.clear([30, 40, 60, 255])
  draw_level(window)
  game[:coins].each { |coin| draw_coin(window, coin) unless coin[:taken] }
  draw_player(window, player)

  window.view = window.default_view
  hud.string = "coins #{game[:score]}/#{COINS.size}   " \
               "#{game[:state] == :won ? 'YOU WIN! R restarts' : 'Left/Right move, Up/Space jump, R restarts'}"
  window.draw(hud)
  window.display

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

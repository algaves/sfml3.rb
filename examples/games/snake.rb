# frozen_string_literal: true

# Snake: a grid game built from RectangleShapes and a Text HUD. Arrow keys (or
# WASD) steer, the snake advances on a fixed timer, and eating grows it. Running
# into a wall or itself ends the run; R restarts, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/snake.rb
# (headless: SFML_EXAMPLE_FRAMES=600 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

CELL = 20
COLS = 32
ROWS = 22
STEP = 0.11

DIRECTIONS = {
  Up: [0, -1], Down: [0, 1], Left: [-1, 0], Right: [1, 0],
  w: [0, -1], s: [0, 1], a: [-1, 0], d: [1, 0]
}.freeze

class Snake
  WIDTH = COLS
  HEIGHT = ROWS

  attr_reader :score, :state

  def initialize
    reset
  end

  def reset
    @snake = [[WIDTH / 2, HEIGHT / 2], [(WIDTH / 2) - 1, HEIGHT / 2], [(WIDTH / 2) - 2, HEIGHT / 2]]
    @direction = [1, 0]
    @pending = []
    @food = place_food
    @score = 0
    @state = :playing
    @clock = Clock.new
  end

  def turn(direction)
    return unless @state == :playing

    @pending << direction
  end

  def update
    return :dead unless @state == :playing
    return :waiting if @clock.elapsed_time.as_seconds < STEP

    @clock.restart!
    apply_pending_turn
    advance
  end

  def draw(window)
    draw_board(window)
    draw_food(window)
    draw_snake(window)
  end

  def to_hud
    "score #{@score}   length #{@snake.length}   " \
      "#{@state == :over ? 'GAME OVER -- R restarts' : 'arrows/WASD move'}"
  end

  private

  def apply_pending_turn
    direction = @pending.shift
    return unless direction

    @direction = direction unless direction[0] == -@direction[0] && direction[1] == -@direction[1]
  end

  def advance
    head = [@snake.first[0] + @direction[0], @snake.first[1] + @direction[1]]
    return die if wall?(head) || @snake.include?(head)

    @snake.unshift(head)
    if head == @food
      @score += 1
      @food = place_food
      :ate
    else
      @snake.pop
      :moved
    end
  end

  def wall?(cell)
    cell[0].negative? || cell[0] >= WIDTH || cell[1].negative? || cell[1] >= HEIGHT
  end

  def die
    @state = :over
    :dead
  end

  def place_food
    loop do
      cell = [rand(WIDTH), rand(HEIGHT)]
      return cell unless @snake.include?(cell)
    end
  end

  def draw_board(window)
    window.draw(ExampleSupport.line(
                  (0..ROWS).flat_map { |row| [[0, row * CELL], [COLS * CELL, row * CELL]] } +
                  (0..COLS).flat_map { |column| [[column * CELL, 0], [column * CELL, ROWS * CELL]] },
                  [34, 38, 50, 255]
                ))
  end

  def draw_food(window)
    food = CircleShape.new((CELL / 2.0) - 2)
    food.origin = [(CELL / 2.0) - 2, (CELL / 2.0) - 2]
    food.position = [(@food[0] * CELL) + (CELL / 2.0), (@food[1] * CELL) + (CELL / 2.0)]
    food.fill_color = [235, 90, 90, 255]
    window.draw(food)
  end

  def draw_snake(window)
    @snake.each_with_index do |cell, index|
      block = RectangleShape.new([CELL - 2, CELL - 2])
      block.position = [(cell[0] * CELL) + 1, (cell[1] * CELL) + 1]
      shade = index.zero? ? [180, 240, 200, 255] : [110, 210, 140, 255]
      block.fill_color = shade
      window.draw(block)
    end
  end
end

window = Window.new(VideoMode.new(COLS * CELL, (ROWS * CELL) + 30, 32), 'SFML snake')
window.frame_rate = 60
game = Snake.new
hud = ExampleSupport.text('', size: 16, position: [8, (ROWS * CELL) + 5])
eat_sound = ExampleSupport.sound('eat')
death_sound = ExampleSupport.sound('explode')
frame = 0

loop do
  restart = false
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      window.close! if key == :escape
      restart = true if key == :r
      direction = DIRECTIONS[key]
      game.turn(direction) if direction
    end
  end
  game.reset if restart

  result = game.update
  eat_sound.play! if result == :ate
  death_sound.play! if result == :dead

  hud.string = game.to_hud
  window.clear!([18, 22, 30, 255])
  game.draw(window)
  window.draw(hud)
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

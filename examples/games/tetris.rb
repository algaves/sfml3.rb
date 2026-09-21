# frozen_string_literal: true

# Tetris: seven tetrominoes from a 7-bag, rotation, a ghost drop, hold and a
# next queue, with line clears scoring by level. Left/Right move, Down soft
# drops, Up or X rotates clockwise, Z counter-clockwise, Space hard drops,
# C holds, P pauses, R restarts, escape returns to the menu.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/tetris.rb
# (headless: SFML_EXAMPLE_FRAMES=900 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

COLS = 10
ROWS = 20
CELL = 30
BOARD_W = COLS * CELL
BOARD_H = ROWS * CELL
SIDEBAR = 180
HUD = 28

SHAPES = {
  i: [[0, 1], [1, 1], [2, 1], [3, 1]],
  o: [[1, 0], [2, 0], [1, 1], [2, 1]],
  t: [[1, 0], [0, 1], [1, 1], [2, 1]],
  s: [[1, 0], [2, 0], [0, 1], [1, 1]],
  z: [[0, 0], [1, 0], [1, 1], [2, 1]],
  j: [[0, 0], [0, 1], [1, 1], [2, 1]],
  l: [[2, 0], [0, 1], [1, 1], [2, 1]]
}.freeze

def rotate_cells(cells)
  height = cells.map { |_, y| y }.max + 1
  rotated = cells.map { |x, y| [height - 1 - y, x] }
  min_x = rotated.map(&:first).min
  min_y = rotated.map(&:last).min
  rotated.map { |x, y| [x - min_x, y - min_y] }
end

ROTATIONS = SHAPES.transform_values do |cells|
  states = [cells]
  3.times { states << rotate_cells(states.last) }
  states.freeze
end.freeze

class Tetris
  attr_reader :score, :lines, :level, :state, :next_queue, :hold_piece, :board, :active

  def initialize
    reset
  end

  def reset
    @board = Array.new(ROWS) { Array.new(COLS) }
    @score = 0
    @lines = 0
    @level = 1
    @state = :playing
    @bag = []
    @next_queue = Array.new(3) { draw_bag }
    @hold_piece = nil
    @hold_used = false
    @drop_timer = 0.0
    @lock_timer = 0.0
    spawn_piece
  end

  def update(delta)
    return unless @state == :playing

    @drop_timer += delta
    return unless @drop_timer >= drop_interval

    @drop_timer = 0.0
    step_down
  end

  def move(dx)
    return unless @state == :playing

    try_move(dx, 0) ? (@lock_timer = 0.0) : nil
  end

  def rotate(direction)
    return false unless @state == :playing

    cells = rotate_cells_index(direction)
    return false unless fits?(cells, @active[:x], @active[:y])

    @active[:rotation] = cells
    @lock_timer = 0.0
    true
  end

  def soft_drop
    return unless @state == :playing

    @score += 1 if try_move(0, 1)
    @drop_timer = 0.0
  end

  def hard_drop
    return unless @state == :playing

    @score += (drop_distance * 2)
    @active[:y] += drop_distance
    lock_piece
  end

  def hold
    return unless @state == :playing
    return if @hold_used

    current = @active[:key]
    if @hold_piece
      swap = @hold_piece
      @hold_piece = current
      spawn_piece(swap)
    else
      @hold_piece = current
      spawn_piece
    end
    @hold_used = true
  end

  def toggle_pause
    @state = @state == :playing ? :paused : :playing unless %i[game_over].include?(@state)
  end

  def ghost_y
    y = @active[:y]
    y += 1 while fits?(@active[:rotation], @active[:x], y + 1)
    y
  end

  private

  def draw_bag
    @bag = SHAPES.keys.shuffle if @bag.empty?
    @bag.shift
  end

  def spawn_piece(key = nil)
    key ||= @next_queue.shift
    @next_queue << draw_bag while @next_queue.length < 3
    @active = { key: key, rotation: ROTATIONS.fetch(key)[0], x: 3, y: 0 }
    @lock_timer = 0.0
    @hold_used = false
    @state = :game_over unless fits?(@active[:rotation], @active[:x], @active[:y])
  end

  def rotate_cells_index(direction)
    states = ROTATIONS.fetch(@active[:key])
    index = states.index(@active[:rotation]) || 0
    states[(index + direction) % states.length]
  end

  def fits?(cells, offset_x, offset_y)
    cells.all? do |x, y|
      board_x = offset_x + x
      board_y = offset_y + y
      board_x.between?(0, COLS - 1) && board_y < ROWS && (board_y.negative? || @board[board_y][board_x].nil?)
    end
  end

  def try_move(dx, dy)
    return false unless fits?(@active[:rotation], @active[:x] + dx, @active[:y] + dy)

    @active[:x] += dx
    @active[:y] += dy
    true
  end

  def drop_distance
    ghost_y - @active[:y]
  end

  def step_down
    if try_move(0, 1)
      @lock_timer = 0.0
    else
      @lock_timer += drop_interval
      lock_piece if @lock_timer >= 0.35
    end
  end

  def lock_piece
    @active[:rotation].each do |x, y|
      board_y = @active[:y] + y
      next if board_y.negative?

      @board[board_y][@active[:x] + x] = @active[:key]
    end
    clear_lines
    spawn_piece
  end

  def clear_lines
    remaining = @board.reject(&:all?)
    cleared = ROWS - remaining.length
    return if cleared.zero?

    @board = Array.new(cleared) { Array.new(COLS) } + remaining
    @lines += cleared
    @score += [0, 100, 300, 500, 800][cleared] * @level
    @level = 1 + (@lines / 10)
  end

  def drop_interval
    [0.9 - ((@level - 1) * 0.07), 0.08].max
  end
end

window = Window.new(VideoMode.new(BOARD_W + SIDEBAR, BOARD_H + HUD, 32), 'SFML tetris')
window.frame_rate = 60
clock = Clock.new
rotate_sound = ExampleSupport.sound('beep', volume: 30)
clear_sound = ExampleSupport.sound('eat', volume: 45)
over_sound = ExampleSupport.sound('explode', volume: 50)
hud = ExampleSupport.text('', size: 16, position: [8, 6])

game = nil
frame = 0
last_lines = 0
last_game_state = nil

start_game = lambda do
  game = Tetris.new
  clock.restart!
  last_lines = 0
  last_game_state = nil
end
start_game.call

def draw_cell(window, sprite_name, column, row, alpha = 255)
  sprite = ExampleSupport.sprite(sprite_name, scale: CELL / 16.0)
  sprite.position = [column * CELL, (row * CELL) + HUD]
  sprite.color = Color.new(255, 255, 255, alpha) if alpha != 255
  window.draw(sprite)
end

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      if key == :escape
        window.close!
      else
        case key
        when :r then start_game.call
        when :Left then game.move(-1)
        when :Right then game.move(1)
        when :Down then game.soft_drop
        when :Up, :x then rotate_sound.play! if game.rotate(1)
        when :z then rotate_sound.play! if game.rotate(-1)
        when :Space then game.hard_drop
        when :c then game.hold
        when :p then game.toggle_pause
        end
      end
    end
  end

  window.clear!([12, 14, 22, 255])

  game.update(clock.restart!.as_seconds)
  clear_sound.play! if game.lines > last_lines
  over_sound.play! if game.state == :game_over && last_game_state != :game_over
  last_lines = game.lines
  last_game_state = game.state

  board_panel = RectangleShape.new([BOARD_W, BOARD_H])
  board_panel.position = [0, HUD]
  board_panel.fill_color = [18, 20, 30, 255]
  window.draw(board_panel)

  game.board.each_with_index do |row, y|
    row.each_with_index do |key, x|
      draw_cell(window, :"tetro_#{key}", x, y) if key
    end
  end

  ghost_y = game.ghost_y
  game.active[:rotation].each do |x, y|
    draw_cell(window, :"tetro_#{game.active[:key]}", game.active[:x] + x, ghost_y + y, 70)
    draw_cell(window, :"tetro_#{game.active[:key]}", game.active[:x] + x, game.active[:y] + y)
  end

  sidebar_x = BOARD_W + 16
  window.draw(ExampleSupport.text('NEXT', size: 16, position: [sidebar_x, HUD + 8]))
  game.next_queue.compact.each_with_index do |key, index|
    ROTATIONS.fetch(key)[0].each do |x, y|
      sprite = ExampleSupport.sprite(:"tetro_#{key}", scale: (CELL - 8) / 16.0)
      sprite.position = [sidebar_x + (x * (CELL - 8)), HUD + 36 + (index * 74) + (y * (CELL - 8))]
      window.draw(sprite)
    end
  end

  hold_y = HUD + 36 + (game.next_queue.compact.length * 74) + 20
  window.draw(ExampleSupport.text('HOLD', size: 16, position: [sidebar_x, hold_y]))
  if game.hold_piece
    ROTATIONS.fetch(game.hold_piece)[0].each do |x, y|
      sprite = ExampleSupport.sprite(:"tetro_#{game.hold_piece}", scale: (CELL - 8) / 16.0)
      sprite.position = [sidebar_x + (x * (CELL - 8)), hold_y + 26 + (y * (CELL - 8))]
      window.draw(sprite)
    end
  end

  hud.string = "score #{game.score}   lines #{game.lines}   level #{game.level}"
  window.draw(hud)

  if game.state == :game_over
    overlay = RectangleShape.new([BOARD_W, 120])
    overlay.position = [0, HUD + (BOARD_H / 2) - 60]
    overlay.fill_color = [0, 0, 0, 190]
    window.draw(overlay)
    window.draw(ExampleSupport.text("GAME OVER\nscore #{game.score}\nR to retry",
                                    size: 26, position: [70, HUD + (BOARD_H / 2) - 50]))
  elsif game.state == :paused
    window.draw(ExampleSupport.text('PAUSED -- P resumes', size: 24, position: [40, HUD + (BOARD_H / 2)]))
  end

  window.display!
  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

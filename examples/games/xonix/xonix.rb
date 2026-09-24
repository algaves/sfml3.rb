# frozen_string_literal: true

# Xonix: carve territory out of the field. Leave the safe border, draw a line
# into the open area, and close the loop to fill everything you enclosed --
# while bouncing enemies try to touch your line. Fill 80% to win; three lives,
# R restarts, escape returns to the menu.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/xonix/xonix.rb
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

SHEET = Texture.from_file(File.join(ASSETS, 'sprites.png'))
SPRITES = { enemy: [4, 2] }.freeze
def sprite(name, scale: 1)
  column, row = SPRITES.fetch(name)
  result = Sprite.new(SHEET)
  result.texture_rect = [column * 16, row * 16, 16, 16]
  result.scale = [scale, scale] if scale != 1
  result
end

CELL = 8
COLS = 90
ROWS = 62
HUD = 36
STEP = 0.045
WIN_PERCENT = 80
EMPTY = 0
FILLED = 1
TRAIL = 2

class XonixGame
  attr_reader :score_percent, :lives, :state

  def initialize
    @triangles = SF::Graphics::VertexArray.new
    @triangles.primitive = :triangles
    @trail_triangles = SF::Graphics::VertexArray.new
    @trail_triangles.primitive = :triangles
    reset
  end

  def reset
    @grid = Array.new(ROWS) { Array.new(COLS, EMPTY) }
    border = []
    ROWS.times { |y| border << [0, y] << [COLS - 1, y] }
    COLS.times { |x| border << [x, 0] << [x, ROWS - 1] }
    border.each { |x, y| @grid[y][x] = FILLED }
    @player = [COLS / 2, 0]
    @drawing = false
    @lives = 3
    @state = :playing
    @enemies = Array.new(4) { spawn_enemy }
    @dirty = true
    @score_percent = 0
  end

  def update(delta, direction)
    return unless @state == :playing

    @timer = (@timer || 0) + delta
    return if @timer < STEP

    @timer -= STEP
    step_player(direction)
    step_enemies(delta)
    update_percent
  end

  def draw(window, _trail = nil)
    if @dirty
      @triangles.clear!
      ROWS.times do |y|
        COLS.times do |x|
          next unless @grid[y][x] == FILLED

          color = ((x + y).even? ? [26, 60, 92, 255] : [22, 50, 80, 255])
          append_cell(@triangles, x, y, color)
        end
      end
      @dirty = false
    end

    @trail_triangles.clear!
    ROWS.times do |y|
      COLS.times do |x|
        append_cell(@trail_triangles, x, y, [120, 220, 255, 255]) if @grid[y][x] == TRAIL
      end
    end

    window.draw(@triangles)
    window.draw(@trail_triangles)

    head = RectangleShape.new([CELL, CELL])
    head.position = [@player[0] * CELL, (@player[1] * CELL) + HUD]
    head.fill_color = [235, 235, 245, 255]
    window.draw(head)

    @enemies.each do |enemy|
      sprite = sprite(:enemy, scale: 1)
      sprite.origin = [8, 8]
      sprite.position = [(enemy[:x] * CELL) + (CELL / 2), (enemy[:y] * CELL) + HUD + (CELL / 2)]
      sprite.scale = [1.5, 1.5]
      window.draw(sprite)
    end
  end

  private

  def spawn_enemy
    { x: rand(4..(COLS - 5)).to_f, y: rand(6..(ROWS - 5)).to_f,
      dx: [-1.0, 1.0].sample * 3.5, dy: [-1.0, 1.0].sample * 3.5 }
  end

  def step_player(direction)
    return unless direction

    target = [@player[0] + direction[0], @player[1] + direction[1]]
    return unless target[0].between?(0, COLS - 1) && target[1].between?(0, ROWS - 1)

    case @grid[target[1]][target[0]]
    when FILLED
      complete_trail if @drawing
      @player = target
      @drawing = false
    when EMPTY
      @grid[target[1]][target[0]] = TRAIL
      @player = target
      @drawing = true
    end
  end

  def step_enemies(delta)
    @enemies.each do |enemy|
      enemy[:x] += enemy[:dx] * delta
      enemy[:y] += enemy[:dy] * delta
      bounce(enemy, :x, :dx, COLS)
      bounce(enemy, :y, :dy, ROWS)

      cell_x = enemy[:x].clamp(0, COLS - 1).to_i
      cell_y = enemy[:y].clamp(0, ROWS - 1).to_i
      kill_player if @grid[cell_y][cell_x] == TRAIL
    end
  end

  def bounce(enemy, axis, velocity, limit)
    return if enemy[axis] > 0.5 && enemy[axis] < limit - 1.5

    enemy[axis] = enemy[axis].clamp(0.5, limit - 1.5)
    enemy[velocity] = -enemy[velocity]
  end

  def complete_trail
    reachable = Array.new(ROWS) { Array.new(COLS, false) }
    queue = []
    @enemies.each do |enemy|
      cell_x = enemy[:x].clamp(0, COLS - 1).to_i
      cell_y = enemy[:y].clamp(0, ROWS - 1).to_i
      next unless @grid[cell_y][cell_x] == EMPTY && !reachable[cell_y][cell_x]

      reachable[cell_y][cell_x] = true
      queue << [cell_x, cell_y]
    end
    until queue.empty?
      x, y = queue.shift
      [[1, 0], [-1, 0], [0, 1], [0, -1]].each do |dx, dy|
        nx = x + dx
        ny = y + dy
        next unless nx.between?(0, COLS - 1) && ny.between?(0, ROWS - 1)
        next unless @grid[ny][nx] == EMPTY && !reachable[ny][nx]

        reachable[ny][nx] = true
        queue << [nx, ny]
      end
    end

    ROWS.times do |y|
      COLS.times do |x|
        @grid[y][x] = FILLED if @grid[y][x] == TRAIL
        @grid[y][x] = FILLED if @grid[y][x] == EMPTY && !reachable[y][x]
      end
    end
    @dirty = true
    @enemies.each do |enemy|
      cell_x = enemy[:x].clamp(0, COLS - 1).to_i
      cell_y = enemy[:y].clamp(0, ROWS - 1).to_i
      next if @grid[cell_y][cell_x] == EMPTY

      enemy.replace(spawn_enemy)
    end
  end

  def kill_player
    @lives -= 1
    if @lives.zero?
      @state = :game_over
      return
    end

    ROWS.times { |y| COLS.times { |x| @grid[y][x] = EMPTY if @grid[y][x] == TRAIL } }
    @player = [COLS / 2, 0]
    @drawing = false
    @dirty = true
  end

  def update_percent
    filled = @grid.sum { |row| row.count(FILLED) }
    @score_percent = (filled * 100.0 / (COLS * ROWS)).round
    @state = :won if @score_percent >= WIN_PERCENT
  end
end

def append_cell(array, x, y, color)
  left = x * CELL
  top = (y * CELL) + HUD
  [[0, 0], [1, 0], [0, 1], [1, 0], [1, 1], [0, 1]].each do |dx, dy|
    array.append(SF::Graphics::Vertex.new([left + (dx * CELL), top + (dy * CELL)], color))
  end
end

window = Window.new(VideoMode.new(COLS * CELL, (ROWS * CELL) + HUD, 32), 'SFML xonix')
window.frame_rate = 60
clock = Clock.new
death_sound = sound('explode', volume: 45)
fill_sound = sound('beep', volume: 40)
hud = text('', size: 15, position: [10, 10])

game = XonixGame.new
last_lives = game.lives
last_game_state = nil
clock.restart!

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      if key == :escape
        window.close!
      elsif key == :r
        game = XonixGame.new
        clock.restart!
        last_lives = game.lives
        last_game_state = nil
      end
    end
  end

  window.clear!([8, 10, 16, 255])

  direction = nil
  direction = [0, -1] if Keyboard.key_pressed?(:Up)
  direction = [0, 1] if Keyboard.key_pressed?(:Down)
  direction = [-1, 0] if Keyboard.key_pressed?(:Left)
  direction = [1, 0] if Keyboard.key_pressed?(:Right)

  game.update(clock.restart!.as_seconds, direction)
  game.draw(window)

  death_sound.play! if game.lives < last_lives
  last_lives = game.lives
  fill_sound.play! if game.state == :won && last_game_state != :won
  last_game_state = game.state

  hud.string = "filled #{game.score_percent}%   lives #{game.lives}   " \
               "#{game.state == :playing ? 'arrows carve, escape quits' : game.state.to_s.upcase}"
  window.draw(hud)

  window.display!
end

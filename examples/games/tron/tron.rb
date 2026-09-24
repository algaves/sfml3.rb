# frozen_string_literal: true

# Tron: two light cycles leave walls behind them and die on any contact. The
# menu picks one player (against a small flood-fill AI) or two (WASD vs arrow
# keys). First to five rounds wins the match; escape returns to the menu.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/tron/tron.rb
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

CELL = 8
COLS = 100
ROWS = 66
HUD = 36
STEP = 0.055
WIN_SCORE = 5
DIRECTIONS = { Up: [0, -1], Down: [0, 1], Left: [-1, 0], Right: [1, 0],
               w: [0, -1], s: [0, 1], a: [-1, 0], d: [1, 0] }.freeze
P1_KEYS = { w: [0, -1], s: [0, 1], a: [-1, 0], d: [1, 0] }.freeze
P2_KEYS = { Up: [0, -1], Down: [0, 1], Left: [-1, 0], Right: [1, 0] }.freeze

def append_cell(array, cell, color)
  x = cell[0] * CELL
  y = (cell[1] * CELL) + HUD
  [[0, 0], [1, 0], [0, 1], [1, 0], [1, 1], [0, 1]].each do |dx, dy|
    array.append(SF::Graphics::Vertex.new([x + (dx * CELL), y + (dy * CELL)], color))
  end
end

class TronGame
  attr_reader :state, :score, :round_winner, :ai, :trails, :cycles

  def initialize(ai:)
    @ai = ai
    @trails = [SF::Graphics::VertexArray.new, SF::Graphics::VertexArray.new]
    @trails.each { |t| t.primitive = :triangles }
    reset_match
  end

  def reset_match
    @score = [0, 0]
    @state = :countdown
    @timer = 0.0
    reset_round
  end

  def reset_round
    @grid = Array.new(ROWS) { Array.new(COLS, 0) }
    @cycles = [
      { cell: [COLS / 4, ROWS / 2], dir: [1, 0], alive: true },
      { cell: [(COLS * 3) / 4, ROWS / 2], dir: [-1, 0], alive: true }
    ]
    @trails.each(&:clear!)
    @cycles.each_with_index { |cycle, index| occupy(cycle[:cell], index) }
    @round_winner = nil
    @state = :countdown
    @timer = 1.0
  end

  def turn(player, direction)
    return unless @state == :running

    current = @cycles[player][:dir]
    @cycles[player][:dir] = direction unless direction == [-current[0], -current[1]]
  end

  def update(delta)
    case @state
    when :countdown
      @timer -= delta
      @state = :running if @timer <= 0
    when :running
      @timer += delta
      return unless @timer >= STEP

      @timer -= STEP
      step
    when :round_over
      @timer -= delta
      if @round_winner
        @state = :match_over
      elsif @timer <= 0
        reset_round
      end
    end
  end

  private

  def step
    @cycles[1][:dir] = ai_direction if @ai && @cycles[1][:alive]
    heads = @cycles.map { |cycle| add(cycle[:cell], cycle[:dir]) }
    survive = heads.map { |head| free?(head) }
    survive = [false, false] if heads[0] == heads[1]

    @cycles.each_with_index do |cycle, index|
      next unless survive[index]

      cycle[:cell] = heads[index]
      occupy(cycle[:cell], index)
    end

    2.times { |index| @cycles[index][:alive] = survive[index] }
    return if survive.all?

    @score[0] += 1 if survive[0] && !survive[1]
    @score[1] += 1 if survive[1] && !survive[0]
    @round_winner = if @score[0] >= WIN_SCORE then 0
                    elsif @score[1] >= WIN_SCORE then 1
                    end
    @state = :round_over
    @timer = 1.6
  end

  def free?(cell)
    x, y = cell
    x.between?(0, COLS - 1) && y.between?(0, ROWS - 1) && @grid[y][x].zero?
  end

  def add(cell, direction)
    [cell[0] + direction[0], cell[1] + direction[1]]
  end

  def occupy(cell, owner)
    x, y = cell
    return unless x.between?(0, COLS - 1) && y.between?(0, ROWS - 1)

    @grid[y][x] = owner + 1
    append_cell(@trails[owner], cell, owner.zero? ? [90, 200, 255, 255] : [255, 150, 90, 255])
  end

  # Picks the neighbouring empty cell that reaches the most open space.
  def ai_direction
    current = @cycles[1][:dir]
    options = DIRECTIONS.values.uniq.reject { |dir| dir == [-current[0], -current[1]] }
    options.max_by { |dir| space_from(add(@cycles[1][:cell], dir)) }
  end

  def space_from(start)
    return -1 unless free?(start)

    seen = { start => true }
    queue = [start]
    count = 0
    until queue.empty? || count >= 1500
      cell = queue.shift
      count += 1
      DIRECTIONS.values.uniq.each do |dir|
        neighbour = add(cell, dir)
        next unless free?(neighbour) && !seen[neighbour]

        seen[neighbour] = true
        queue << neighbour
      end
    end
    count
  end
end

window = Window.new(VideoMode.new(COLS * CELL, (ROWS * CELL) + HUD, 32), 'SFML tron')
window.frame_rate = 60
clock = Clock.new
crash_sound = sound('explode', volume: 45)
score_sound = sound('beep', volume: 45)
hud = text('', size: 15, position: [10, 10])

# Two players by default (WASD vs arrow keys); M swaps the second for the AI.
game = TronGame.new(ai: false)
last_total = 0
last_state = nil
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
      elsif key == :m
        game = TronGame.new(ai: !game.ai)
        clock.restart!
        last_total = 0
        last_state = nil
      elsif key == :r
        game.reset_match
        clock.restart!
        last_total = 0
        last_state = nil
      elsif %i[running countdown].include?(game.state)
        game.turn(0, DIRECTIONS[key]) if P1_KEYS.key?(key)
        game.turn(1, DIRECTIONS[key]) if P2_KEYS.key?(key) && !game.ai
      end
    end
  end

  window.clear!([10, 12, 20, 255])

  game.update(clock.restart!.as_seconds)
  total = game.score.sum
  score_sound.play! if total > last_total
  crash_sound.play! if game.state == :round_over && last_state != :round_over
  last_total = total
  last_state = game.state

  game.trails.each { |trail| window.draw(trail) }
  game.cycles.each_with_index do |cycle, index|
    head = RectangleShape.new([CELL, CELL])
    head.position = [cycle[:cell][0] * CELL, (cycle[:cell][1] * CELL) + HUD]
    head.fill_color = index.zero? ? [200, 240, 255, 255] : [255, 210, 170, 255]
    window.draw(head)
  end
  hud.string = if game.round_winner
                 "player #{game.round_winner + 1} wins the match!   R restarts, M changes mode, escape quits"
               else
                 "#{game.state}   P1 (WASD) #{game.score[0]}  -  #{game.score[1]} P2 " \
                   "#{game.ai ? '(computer)' : '(arrows)'}   M changes mode, escape quits"
               end
  window.draw(hud)

  window.display!
end

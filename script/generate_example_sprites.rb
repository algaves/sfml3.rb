# frozen_string_literal: true

# Regenerates examples/assets/sprites.png, the only image the examples bundle.
# It draws each cell with SF::Graphics::Image so the art is reproducible from the
# binding itself (no ImageMagick dependency):
#
#   bundle exec ruby -Ilib script/generate_example_sprites.rb
#
# The cell layout is mirrored in ExampleSupport::SPRITES (examples/support.rb);
# keep the two in sync.

require_relative '../examples/support'

CELL = ExampleSupport::SHEET_CELL
COLUMNS = ExampleSupport::SHEET_COLUMNS
ROWS = 8
SIZE = CELL * COLUMNS

# A 16x16 drawing surface offset to one cell of the sheet.
class Cell
  def initialize(image, ox, oy)
    @image = image
    @ox = ox
    @oy = oy
  end

  def color(r, g, b, a = 255)
    SF::Graphics::Color.new(r, g, b, a)
  end

  def pixel(x, y, tint)
    return if x.negative? || y.negative? || x >= CELL || y >= CELL

    @image.set_pixel(@ox + x, @oy + y, tint)
  end

  def rect(x, y, width, height, tint)
    height.times { |dy| width.times { |dx| pixel(x + dx, y + dy, tint) } }
  end

  def disc(cx, cy, radius, tint)
    (-radius..radius).each do |dy|
      (-radius..radius).each do |dx|
        pixel(cx + dx, cy + dy, tint) if (dx * dx) + (dy * dy) <= radius * radius
      end
    end
  end

  def diamond(cx, cy, radius, tint)
    (-radius..radius).each do |dy|
      (-radius..radius).each do |dx|
        pixel(cx + dx, cy + dy, tint) if dx.abs + dy.abs <= radius
      end
    end
  end

  def triangle(x, y, width, height, tint)
    height.times do |dy|
      half = (width * (height - dy) / height.to_f).round
      (((width - half) / 2)...((width + half) / 2)).each { |dx| pixel(x + dx, y + dy, tint) }
    end
  end
end

YELLOW = [250, 220, 70].freeze
ORANGE = [245, 150, 50].freeze
GREEN = [110, 200, 120].freeze
DARK_GREEN = [58, 150, 82].freeze
CYAN = [90, 220, 230].freeze
RED = [222, 82, 82].freeze
BLUE = [80, 130, 230].freeze
PURPLE = [172, 102, 222].freeze
GRAY = [150, 155, 170].freeze
DARK_GRAY = [66, 72, 92].freeze
WHITE = [245, 245, 250].freeze
BROWN = [150, 100, 60].freeze
INK = [30, 32, 44].freeze

TETRO_COLORS = {
  tetro_i: CYAN, tetro_o: YELLOW, tetro_t: PURPLE, tetro_s: GREEN,
  tetro_z: RED, tetro_j: BLUE, tetro_l: ORANGE
}.freeze

def draw(name, cell)
  case name
  when :bird_up, :bird_mid, :bird_down
    wing_y = { bird_up: 4, bird_mid: 8, bird_down: 12 }[name]
    cell.disc(8, 9, 6, YELLOW)
    cell.disc(6, wing_y, 3, ORANGE)
    cell.pixel(11, 7, INK)
    cell.triangle(12, 8, 4, 5, ORANGE)
  when :pipe
    cell.rect(2, 0, 12, 16, DARK_GREEN)
    cell.rect(3, 0, 10, 16, GREEN)
    cell.rect(4, 0, 2, 16, [160, 225, 165])
  when :platform
    cell.rect(0, 5, 16, 7, GREEN)
    cell.rect(0, 5, 16, 2, [160, 225, 165])
    cell.rect(0, 12, 16, 1, BROWN)
  when :gem
    cell.diamond(8, 8, 7, CYAN)
    cell.diamond(8, 8, 4, [180, 245, 250])
  when :car
    cell.rect(1, 3, 13, 9, RED)
    cell.rect(3, 4, 9, 4, DARK_GRAY)
    cell.disc(4, 13, 2, INK)
    cell.disc(11, 13, 2, INK)
    cell.rect(1, 11, 2, 2, YELLOW)
  when :doodler
    cell.disc(8, 8, 5, GREEN)
    cell.rect(5, 2, 2, 3, [180, 245, 190])
    cell.rect(9, 2, 2, 3, [180, 245, 190])
    cell.disc(10, 7, 1, WHITE)
    cell.pixel(10, 7, INK)
    cell.rect(12, 7, 3, 2, ORANGE)
  when :spring
    cell.rect(3, 3, 10, 2, GRAY)
    3.times { |index| cell.rect(4, 5 + (index * 3), 8, 2, ORANGE) }
    cell.rect(3, 13, 10, 2, GRAY)
  when :coin
    cell.disc(8, 8, 7, [200, 150, 40])
    cell.disc(8, 8, 5, YELLOW)
  when :brick_red, :brick_blue
    base = name == :brick_red ? RED : BLUE
    cell.rect(0, 0, 16, 16, base)
    cell.rect(0, 7, 16, 1, [220, 220, 220])
    cell.rect(7, 0, 1, 7, [220, 220, 220])
    cell.rect(3, 8, 1, 8, [220, 220, 220])
    cell.rect(12, 8, 1, 8, [220, 220, 220])
  when :grass
    cell.rect(0, 0, 16, 16, DARK_GREEN)
    6.times { |index| cell.rect((index * 5) % 14, (index * 7) % 14, 2, 2, [130, 215, 140]) }
    cell.rect(0, 12, 16, 4, BROWN)
  when :enemy
    cell.disc(8, 8, 6, PURPLE)
    [-6, -2, 2, 6].each { |x| cell.triangle(x + 6, 1, 4, 4, PURPLE) }
    cell.disc(6, 7, 1, WHITE)
    cell.disc(10, 7, 1, WHITE)
  when :star
    cell.disc(8, 8, 6, YELLOW)
    cell.triangle(2, 0, 12, 8, YELLOW)
    cell.triangle(2, 8, 12, 8, YELLOW)
    cell.disc(8, 8, 3, ORANGE)
  when :block
    cell.rect(0, 0, 16, 16, GRAY)
    cell.rect(1, 1, 14, 14, [190, 194, 205])
    cell.rect(3, 3, 4, 4, GRAY)
  when :arrow
    cell.triangle(2, 0, 12, 16, ORANGE)
  end
end

def draw_tetromino(name, cell)
  tint = TETRO_COLORS.fetch(name)
  cell.rect(2, 2, 12, 12, tint)
  cell.rect(2, 2, 12, 2, [tint[0] + 30, tint[1] + 30, tint[2] + 30].map { |v| [v, 255].min })
  cell.rect(2, 12, 12, 2, tint.map { |v| (v * 0.7).round })
  cell.rect(2, 4, 2, 8, tint.map { |v| (v * 0.8).round })
end

image = SF::Graphics::Image.from_color([SIZE, SIZE], SF::Graphics::Color.new(0, 0, 0, 0))

(0...COLUMNS).each do |column|
  (0...ROWS).each do |row|
    cell = Cell.new(image, column * CELL, row * CELL)
    name = ExampleSupport::SPRITES.key([column, row])
    next unless name

    if name.start_with?('tetro_')
      draw_tetromino(name, cell)
    else
      draw(name, cell)
    end
  end
end

path = ExampleSupport.asset(ExampleSupport::SHEET_NAME)
image.save_to_file(path)
puts "wrote #{path} (#{image.size.x.to_i}x#{image.size.y.to_i})"

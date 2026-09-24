# frozen_string_literal: true

# A tileset and a tile map. The 8-tile terrain atlas is drawn into an Image at
# runtime (grass, dirt, water, sand, stone, path, tree, flower), uploaded once,
# and a deterministic map is rendered two ways: one Sprite per tile, or a single
# batched VertexArray of textured quads. B toggles the two, the arrow keys pan
# the View, the wheel zooms, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/tileset/tileset.rb
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

TILE = 16
TILE_KINDS = 8
ATLAS_WIDTH = TILE * TILE_KINDS

COLORS = {
  grass: [84, 160, 92], dirt: [150, 108, 66], water: [58, 112, 198],
  sand: [224, 202, 122], stone: [128, 130, 142], path: [176, 152, 108],
  trunk: [110, 76, 48], leaf: [58, 132, 74], petal: [240, 180, 90]
}.freeze

# Paints one 16x16 tile into +image+ at the (ox, oy) cell origin.
def paint_tile(image, ox, oy, kind)
  put = lambda do |x, y, color|
    image.set_pixel(ox + x, oy + y, Color.new(*color)) if x.between?(0, TILE - 1) && y.between?(0, TILE - 1)
  end
  fill = lambda do |color|
    TILE.times { |y| TILE.times { |x| put.call(x, y, color) } }
  end
  speckle = lambda do |color, step, limit|
    TILE.times do |y|
      TILE.times do |x|
        put.call(x, y, color) if (((x * 7) + (y * 13)) % step).zero? && (x + y) < limit
      end
    end
  end

  case kind
  when :grass
    fill.call(COLORS[:grass])
    speckle.call([120, 190, 120], 5, 40)
  when :dirt
    fill.call(COLORS[:dirt])
    speckle.call([120, 84, 50], 7, 32)
  when :water
    fill.call(COLORS[:water])
    (0...TILE).each do |x|
      put.call(x, 4 + ((x / 2) % 3), [130, 180, 240])
      put.call(x, 11 + ((x / 3) % 2), [130, 180, 240])
    end
  when :sand
    fill.call(COLORS[:sand])
    speckle.call([200, 178, 100], 6, 36)
  when :stone
    fill.call(COLORS[:stone])
    (0...TILE).each { |i| put.call(i, (i * 3) % TILE, [96, 98, 110]) }
  when :path
    fill.call(COLORS[:path])
    speckle.call([150, 128, 88], 4, 24)
  when :tree
    fill.call(COLORS[:grass])
    (4...12).each { |y| (5...11).each { |x| put.call(x, y, COLORS[:leaf]) } }
    (7...9).each { |x| (11...15).each { |y| put.call(x, y, COLORS[:trunk]) } }
  when :flower
    fill.call(COLORS[:grass])
    [[4, 5], [10, 8], [7, 11]].each do |x, y|
      put.call(x, y, COLORS[:petal])
      put.call(x + 1, y, COLORS[:petal])
      put.call(x, y + 1, COLORS[:petal])
    end
  end
end

TILE_ORDER = %i[grass dirt water sand stone path tree flower].freeze

def build_tileset
  atlas = Image.from_color([ATLAS_WIDTH, TILE], Color.new(0, 0, 0, 0))
  TILE_ORDER.each_with_index { |kind, index| paint_tile(atlas, index * TILE, 0, kind) }
  atlas
end

COLS = 40
ROWS = 28

# grass 0, dirt 1, water 2, sand 3, stone 4, path 5, tree 6, flower 7
def build_map
  rows = Array.new(ROWS) { Array.new(COLS, 0) }
  ROWS.times do |y|
    COLS.times do |x|
      dx = x - 30
      dy = y - 18
      lake = ((dx * dx) / 42.0) + ((dy * dy) / 15.0)
      rows[y][x] = 2 if lake < 1.0
      rows[y][x] = 3 if lake >= 1.0 && lake < 1.35

      path_y = 8 + (Math.sin(x / 5.0) * 3).round
      rows[y][x] = 5 if rows[y][x] < 2 && y == path_y
      rows[y][x] = 4 if x.between?(6, 12) && y.between?(21, 25)

      seed = ((x * 73_856_093) ^ (y * 19_349_663)) % 100
      rows[y][x] = 6 if rows[y][x].zero? && seed < 4
      rows[y][x] = 7 if rows[y][x].zero? && seed.between?(9, 11)
    end
  end
  rows
end

WIDTH = 900
HEIGHT = 560
WORLD_WIDTH = COLS * TILE
WORLD_HEIGHT = ROWS * TILE

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML tileset & tile map')
window.frame_rate = 60

atlas_texture = Texture.from_image(build_tileset)
atlas_texture.smooth = false

map = build_map

# The map as one batched VertexArray of textured quads.
def build_batch(map)
  vertices = VertexArray.new
  vertices.primitive = :quads
  white = [255, 255, 255, 255]
  map.each_with_index do |row, y|
    row.each_with_index do |kind, x|
      left = x * TILE
      top = y * TILE
      u = kind * TILE
      vertices.append(Vertex.new([left, top], white, [u, 0]))
      vertices.append(Vertex.new([left + TILE, top], white, [u + TILE, 0]))
      vertices.append(Vertex.new([left + TILE, top + TILE], white, [u + TILE, TILE]))
      vertices.append(Vertex.new([left, top + TILE], white, [u, TILE]))
    end
  end
  vertices
end

batch = build_batch(map)

sprite = Sprite.new(atlas_texture)
sprite.origin = [0, 0]

view = View.from_rect(Rect.new(0, 0, WIDTH, HEIGHT))
view.center = [WORLD_WIDTH / 2, WORLD_HEIGHT / 2]

batched = true
pan = 260.0

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :b then batched = !batched
      end
    when 'mouse-wheel-scrolled'
      view.zoom(event.mouse_wheel_scroll[:delta].positive? ? 0.9 : 1.1)
    end
  end

  center = view.center
  dx = 0.0
  dy = 0.0
  dx -= pan / 60 if Keyboard.key_pressed?(:Left)
  dx += pan / 60 if Keyboard.key_pressed?(:Right)
  dy -= pan / 60 if Keyboard.key_pressed?(:Up)
  dy += pan / 60 if Keyboard.key_pressed?(:Down)
  view.center = [center.x + dx, center.y + dy]
  window.view = view

  window.clear!([14, 18, 28, 255])

  if batched
    state = RenderState.new
    state.texture = atlas_texture
    window.draw(batch, state)
  else
    map.each_with_index do |row, y|
      row.each_with_index do |kind, x|
        sprite.texture_rect = [kind * TILE, 0, TILE, TILE]
        sprite.position = [x * TILE, y * TILE]
        window.draw(sprite)
      end
    end
  end

  # HUD is drawn in the window's own coordinates, independent of the map view.
  window.view = window.default_view
  size = view.size
  window.draw(text(
                "tileset: #{TILE_KINDS} tiles   map: #{COLS}x#{ROWS}   " \
                "mode: #{batched ? 'batched VertexArray' : 'per-tile Sprite'}   " \
                "view: #{size.x.to_i}x#{size.y.to_i} at #{view.center.x.to_i},#{view.center.y.to_i}\n" \
                'B toggle rendering, arrows pan, wheel zoom, escape quits',
                size: 15, position: [24, 24]
              ))
  window.display!

end

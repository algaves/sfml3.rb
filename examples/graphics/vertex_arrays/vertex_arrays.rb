# frozen_string_literal: true

# A VertexArray is an ordered list of vertices plus a primitive that says how to
# connect them: points, lines, line strips, triangles, triangle strips and
# triangle fans. This recipe builds one ring of coloured vertices and re-reads
# it as each primitive, then switches to a single VertexArray holding thousands
# of triangles drawn in one call -- the batching that makes large scenes cheap.
#
# Press 1-6 to pick a primitive, B to toggle batching, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/vertex_arrays/vertex_arrays.rb
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

PRIMITIVES = %i[points lines line_strip triangles triangle_strip triangle_fan].freeze

WIDTH = 900
HEIGHT = 560
CENTER_X = WIDTH / 2
CENTER_Y = (HEIGHT / 2) + 20
COUNT = 12

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML vertex arrays: primitives & batching')
window.frame_rate = 60

# One ring of vertices; only the primitive changes how they are joined.
ring = VertexArray.new
COUNT.times do |i|
  angle = ((i * 2 * Math::PI) / COUNT) - (Math::PI / 2)
  x = CENTER_X + (Math.cos(angle) * 170)
  y = CENTER_Y + (Math.sin(angle) * 170)
  hue = (255.0 * i / COUNT).round
  ring.append(Vertex.new([x, y], [hue, 255 - hue, 180, 255]))
end

# A single batched array of 1500 small triangles -- one draw call for thousands
# of vertices. Built once, then re-drawn every frame.
batch = VertexArray.new
batch.primitive = :triangles
srand(11)
1500.times do
  x = rand(WIDTH)
  y = rand(HEIGHT)
  size = rand(2..5)
  color = [rand(80..255), rand(80..255), rand(80..255), 200]
  batch.append(Vertex.new([x, y], color))
  batch.append(Vertex.new([x + size, y + size], color))
  batch.append(Vertex.new([x + (size * 2), y], color))
end

index = 3
batched = false

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :b then batched = !batched
      when :num1 then index = 0
      when :num2 then index = 1
      when :num3 then index = 2
      when :num4 then index = 3
      when :num5 then index = 4
      when :num6 then index = 5
      end
    end
  end

  window.clear!([16, 18, 28, 255])

  if batched
    window.draw(batch)
    window.draw(text(
                  "batched: #{batch.vertex_count} vertices in one VertexArray, one draw\n" \
                  'B back to primitives, escape quits',
                  size: 15
                ))
  else
    ring.primitive = PRIMITIVES[index]
    window.draw(ring)
    window.draw(text(
                  "primitive: #{PRIMITIVES[index]}   vertices: #{ring.vertex_count}   " \
                  "vertex #{COUNT - 1} = #{ring.vertex(COUNT - 1).position.to_a.join(', ')}\n" \
                  '1-6 choose, B batch 1500 triangles, escape quits',
                  size: 15
                ))
  end
  window.display!
end

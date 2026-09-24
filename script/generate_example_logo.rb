# frozen_string_literal: true

# Writes the small "SF" logo used by the GUI example to
# examples/subsystems/gui/assets/logo.png. Pure Ruby (no SFML, no gems): it
# builds the RGBA pixels, encodes a PNG by hand and deflates the scanlines with
# zlib, so it runs anywhere Ruby does.
#
#   ruby script/generate_example_logo.rb

require 'zlib'

SIZE = 96
RADIUS = 16

# A 5x7 bitmap font, enough to spell SF. 1 = ink.
FONT = {
  'S' => %w[
    01111
    10000
    10000
    01110
    00001
    00001
    11110
  ],
  'F' => %w[
    11111
    10000
    10000
    11110
    10000
    10000
    10000
  ]
}.freeze

def blank_canvas
  Array.new(SIZE) { Array.new(SIZE) { [0, 0, 0, 0] } }
end

def in_corner?(x, y)
  cx = if x < RADIUS
         RADIUS
       else
         (x >= SIZE - RADIUS ? SIZE - RADIUS - 1 : x)
       end
  cy = if y < RADIUS
         RADIUS
       else
         (y >= SIZE - RADIUS ? SIZE - RADIUS - 1 : y)
       end
  dx = x - cx
  dy = y - cy
  (dx * dx) + (dy * dy) > RADIUS * RADIUS
end

def paint_background(canvas)
  SIZE.times do |y|
    SIZE.times do |x|
      next if in_corner?(x, y)

      # A subtle vertical gradient so the logo is not a flat square.
      shade = 30 + ((y * 22) / SIZE)
      canvas[y][x] = [shade, shade + 6, shade + 18, 255]
    end
  end
end

def paint_text(canvas, text, scale, ox, oy, color)
  cursor = ox
  text.each_char do |char|
    rows = FONT.fetch(char)
    rows.each_with_index do |row, gy|
      row.each_char.with_index do |cell, gx|
        next unless cell == '1'

        scale.times do |sy|
          scale.times do |sx|
            x = cursor + (gx * scale) + sx
            y = oy + (gy * scale) + sy
            canvas[y][x] = color if x.between?(0, SIZE - 1) && y.between?(0, SIZE - 1)
          end
        end
      end
    end
    cursor += (rows.first.length * scale) + scale
  end
end

def paint_accent(canvas)
  (20...76).each do |x|
    (74...80).each { |y| canvas[y][x] = [90, 170, 230, 255] }
  end
end

def chunk(type, data)
  [data.bytesize].pack('N') + type + data + [Zlib.crc32(type + data)].pack('N')
end

def png(canvas)
  raw = canvas.map { |row| "\x00#{row.flatten.pack('C*')}" }.join
  ihdr = [SIZE, SIZE, 8, 6, 0, 0, 0].pack('NNC5')
  "\x89PNG\r\n\x1A\n".b + chunk('IHDR', ihdr) + chunk('IDAT', Zlib::Deflate.deflate(raw)) + chunk('IEND', '')
end

canvas = blank_canvas
paint_background(canvas)
paint_text(canvas, 'SF', 5, 20, 26, [235, 235, 245, 255])
paint_accent(canvas)

path = File.expand_path('../examples/subsystems/gui/assets/logo.png', __dir__)
File.binwrite(path, png(canvas))
puts "wrote #{path} (#{File.size(path)} bytes)"

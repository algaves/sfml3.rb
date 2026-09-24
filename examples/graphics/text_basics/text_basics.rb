# frozen_string_literal: true

# Everything Text and Font can do, side by side: sizes, the five styles (or any
# combination), fill and outline, letter and line spacing, centring from the
# local bounds, and the measuring helpers (`local_bounds`, `global_bounds`,
# `find_character_pos`, `Font#glyph`, `#has_glyph?`, `#kerning`, `#line_spacing`).
# +/- change the size, S cycles the style, L toggles letter spacing, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/text_basics/text_basics.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255], style: nil)
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label.style = style if style
  label
end

WIDTH = 900
HEIGHT = 620

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML text & fonts')
window.frame_rate = 60

SENTENCE = 'The quick brown fox jumps over the lazy dog'
STYLES = [:regular, :bold, :italic, %i[bold italic], [:underlined], [:strike_through]].freeze

sample = text(SENTENCE, size: 22, position: [40, 96])
style_index = 0
spaced = false

sizes = [12, 18, 24, 36].map.with_index do |size, index|
  text("size #{size}", size: size, position: [40, 150 + (index * 52)], color: [150, 200, 255, 255])
end

outlined = text('outlined', size: 30, position: [430, 150], style: :bold)
outlined.outline_thickness = 2
outlined.outline_color = [255, 210, 90, 255]

multiline = text("one\ntwo\nthree", size: 20, position: [430, 210], color: [120, 230, 160, 255])

# Centre a label in a box using its measured local bounds as the origin.
box = RectangleShape.new([260, 90])
box.position = [580, 90]
box.fill_color = [26, 30, 44, 255]
box.outline_thickness = 1
box.outline_color = [120, 128, 150, 255]
centred = text('centred', size: 24, position: [580 + 130, 90 + 45])
bounds = centred.local_bounds
centred.origin = [bounds.size.x / 2, bounds.size.y / 2]

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :s
        style_index = (style_index + 1) % STYLES.length
        sample.style = STYLES[style_index]
      when :l
        spaced = !spaced
        sample.letter_spacing = spaced ? 4.0 : 0.0
        multiline.line_spacing = spaced ? 1.6 : 1.0
      when :Equal, :Add
        sample.character_size = [sample.character_size + 2, 48].min
        centred.character_size = sample.character_size
        b = centred.local_bounds
        centred.origin = [b.size.x / 2, b.size.y / 2]
      when :Hyphen, :Subtract
        sample.character_size = [sample.character_size - 2, 8].max
        centred.character_size = sample.character_size
        b = centred.local_bounds
        centred.origin = [b.size.x / 2, b.size.y / 2]
      end
    end
  end

  measured = sample.local_bounds
  glyph = FONT.glyph('A'.ord, sample.character_size)
  caret = sample.find_character_pos(4)
  kerning = FONT.kerning('A'.ord, 'V'.ord, sample.character_size)
  line_spacing = FONT.line_spacing(sample.character_size)

  window.clear!([18, 20, 30, 255])

  window.draw(text('Text & Font', size: 16, position: [40, 40], color: [120, 128, 150, 255]))
  window.draw(box)
  sizes.each { |label| window.draw(label) }
  window.draw(outlined)
  window.draw(multiline)
  window.draw(centred)
  window.draw(sample)

  window.draw(text(
                "style: #{STYLES[style_index].inspect}   size: #{sample.character_size}   " \
                "letter_spacing: #{sample.letter_spacing}   line_spacing: #{multiline.line_spacing}\n" \
                "local_bounds: #{measured.size.x.round}x#{measured.size.y.round}   " \
                "global_bounds: #{sample.global_bounds.size.x.round}x#{sample.global_bounds.size.y.round}   " \
                "find_character_pos(4): #{caret.x.round},#{caret.y.round}\n" \
                "glyph('A'): #{glyph.bounds.width.round}x#{glyph.bounds.height.round}   " \
                "has_glyph?('e'): #{FONT.has_glyph?('e'.ord)}   kerning(A,V): #{kerning}   " \
                "line_spacing(font): #{line_spacing.round(1)}\n" \
                '+/- size, S style, L spacing, escape quits',
                size: 14, position: [40, 520]
              ))
  window.display!
end

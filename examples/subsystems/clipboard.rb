# frozen_string_literal: true

# SF::Window::Clipboard: read and write the system clipboard. C copies a sample that
# includes non-ASCII text through `Clipboard.content=` and `unicode_string=`;
# V reads it back with `Clipboard.content` and renders it. The two entry points
# round-trip Unicode exactly, so emoji and CJK survive the trip. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/clipboard.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

SAMPLE = "SFML clipboard \u2014 caf\u00e9, \u4e16\u754c, \u2728"

def wrap(text, width)
  text.split("\n", -1).flat_map do |line|
    chunks = line.scan(/.{1,#{width}}/)
    chunks.empty? ? [''] : chunks
  end
end

window = Window.new(VideoMode.new(640, 360, 32), 'SFML clipboard')
window.frame_rate = 60
frame = 0
pasted = '(press V to paste)'

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      window.close! if key == :escape
      if key == :c
        Clipboard.content = SAMPLE
        Clipboard.unicode_string = SAMPLE
        pasted = "copied #{SAMPLE.length} chars (press V to read it back)"
      elsif key == :v
        pasted = Clipboard.content
      end
    end
  end

  window.clear!([22, 26, 34, 255])
  window.draw(ExampleSupport.text(
                "C copies: #{SAMPLE}\nV pastes. Clipboards round-trip text exactly.\n" \
                'escape quits',
                size: 17
              ))

  panel = RectangleShape.new([window.size.x - 40, 180])
  panel.position = [20, 120]
  panel.fill_color = [34, 40, 54, 255]
  panel.outline_thickness = 1
  panel.outline_color = [90, 100, 125, 255]
  window.draw(panel)

  wrap(pasted, 46).first(7).each_with_index do |line, index|
    window.draw(ExampleSupport.text(line, size: 16, position: [34, 134 + (index * 22)]))
  end

  window.draw(ExampleSupport.text("clipboard length: #{pasted.length}", size: 14, position: [20, 312]))
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

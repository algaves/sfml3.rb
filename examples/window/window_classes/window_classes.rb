# frozen_string_literal: true

# The three window classes side by side. WindowBase is an OS window and event
# queue with no OpenGL context -- it shows but cannot clear, draw or display.
# Window adds the context and stays directly renderable (clear!/display!/draw).
# RenderWindow adds the full RenderTarget surface. The demo opens all three,
# animates the two renderable ones, and retitles the WindowBase each frame with
# its live size. Closing any window (or escape) closes all three.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/window/window_classes/window_classes.rb
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

puts 'SF::Window::WindowBase   : open?, poll_events!, size/title/position, no GL, no draw'
puts 'SF::Window::Window       : WindowBase + GL context: clear!, display!, draw, view, render!'
puts 'SF::Graphics::RenderWindow: Window + RenderTarget: same draw surface, plus render targets'
puts

mode = -> { VideoMode.new(320, 240, 32) }

# No block: the window is returned open, so all three can coexist.
base = WindowBase.open(mode.call, 'WindowBase - events only')
window = Window.open(mode.call, 'Window - GL context')
render = RenderWindow.open(mode.call, 'RenderWindow - RenderTarget')

base.position = [40, 200]
window.position = [400, 200]
render.position = [760, 200]

windows = [base, window, render]

shape = CircleShape.new(60)
shape.origin = [60, 60]
shape.fill_color = [90, 170, 230, 255]
shape.outline_thickness = 3
shape.outline_color = [255, 255, 255, 255]

rotation = 0.0

until windows.any? { |w| !w.open? }
  windows.each do |current|
    current.poll_events! do |event|
      closed = event.type == 'closed'
      escape = event.type == 'key-pressed' && event.code == :escape
      windows.each(&:close!) if closed || escape
    end
  end

  rotation += 2

  # Window: a GL context, so it renders directly.
  window.clear!([18, 22, 34, 255])
  shape.rotation = rotation
  window.draw(shape)
  window.draw(text("SF::Window\nclear! + draw + display!", size: 16, position: [16, 14]))
  window.display!

  # RenderWindow: the same surface, as a RenderTarget.
  render.clear!([24, 18, 30, 255])
  shape.rotation = -rotation
  render.draw(shape)
  render.draw(text("RenderWindow\na RenderTarget", size: 16, position: [16, 14]))
  render.display!

  # WindowBase has no GL context: nothing to draw, so it only reports state.
  base.title = "WindowBase #{base.size.x.to_i}x#{base.size.y.to_i} #{base.focused? ? 'focused' : 'unfocused'}"
end

windows.each(&:close!)

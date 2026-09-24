# frozen_string_literal: true

# Rubyesque (Matz-like): the window. Shows the scoped constructor (`Window.open`), block event
# polling, the one-call `render!` frame and the `open?`/`focused?`/`visible?`
# predicates. V toggles the window's visibility, F asks the window manager for
# focus, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/rubyesque/window/window.rb
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

# `WindowBase.open` / `Window.open` / `RenderWindow.open` create the window,
# yield it and close it when the block returns -- normally or by raising -- so
# there is no `close!` to forget. Without a block they return the open window
# for the caller to manage.
Window.open(VideoMode.new(640, 400, 32), 'SFML rubyesque: window') do |window|
  window.frame_rate = 60

  square = RectangleShape.new([90, 90])
  square.origin = [45, 45]
  square.position = [470, 200]
  square.fill_color = [90, 170, 230, 255]

  angle = 0

  while window.open?
    # With a block, `poll_events!` yields every pending event and returns self.
    # Without one it returns an Enumerator, so these also work:
    #   window.poll_events!.to_a
    #   window.poll_events!.select(&:key_pressed?)
    window.poll_events! do |event|
      case event.type
      when 'closed'
        window.close!
      when 'key-pressed'
        case event.code
        when :escape then window.close!
        when :v then window.visible = !window.visible?
        when :f then window.request_focus!
        end
      end
    end

    square.rotate(1)
    angle += 1

    # `render!` clears to a colour, yields the window to draw into, then
    # presents -- the whole frame in one call. It is short for
    # `clear!` + draw + `display!`.
    window.render!(clear_color: [28, 28, 38, 255]) do |target|
      target.draw(square)
      target.draw(text(
                    "open?     #{window.open?}\n" \
                    "focused?  #{window.focused?}\n" \
                    "visible?  #{window.visible?}\n" \
                    "rotation  #{angle}\n\n" \
                    'V toggles visibility, F requests focus, escape quits',
                    size: 16, position: [18, 16]
                  ))
    end

  end
end

# frozen_string_literal: true

# poll_event! vs wait_event!. In poll mode the loop never stops: it drains
# whatever is queued with `poll_event!` / `poll_events!` and redraws every frame,
# so animation keeps running. In wait mode the loop blocks on `wait_event!`
# until an event arrives, redrawing only then -- ideal for editors and menus
# where nothing changes in between, and wasteful for a game. The idle timer shows
# how long the wait lasted. W toggles the mode, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/event_wait/event_wait.rb
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

WIDTH = 820
HEIGHT = 520

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML events: poll vs wait')
window.frame_rate = 60

bouncer = RectangleShape.new([60, 60])
bouncer.fill_color = [90, 170, 230, 255]
bouncer.position = [100, HEIGHT / 2]

mode = :poll
direction = 1
events_seen = 0
last_wait = 0.0
idle = Clock.new

while window.open?
  handle = lambda do |event|
    events_seen += 1
    next unless event.type == 'key-pressed'
    next unless event.code == :w

    mode = mode == :poll ? :wait : :poll
  end

  if mode == :poll
    # Drain every queued event, then run a normal frame.
    window.poll_events! do |event|
      window.close! if event.type == 'closed' || (event.type == 'key-pressed' && event.code == :escape)
      handle.call(event)
    end
  else
    # Block until exactly one event arrives, then run a single frame.
    idle.restart!
    window.wait_event! do |event|
      window.close! if event.type == 'closed' || (event.type == 'key-pressed' && event.code == :escape)
      handle.call(event)
    end
    last_wait = idle.elapsed_time.as_seconds
  end

  bouncer.position = [bouncer.position.x + (direction * 5), bouncer.position.y]
  direction *= -1 if bouncer.position.x > WIDTH - 80 || bouncer.position.x < 20

  window.clear!([18, 20, 30, 255])
  window.draw(bouncer)
  window.draw(text(
                "mode: #{mode}   events handled: #{events_seen}   " \
                "last wait: #{mode == :wait ? "#{last_wait.round(3)}s" : '-'}\n" \
                "The square only moves when a frame is drawn. In wait mode press any key to wake it.\n" \
                'W toggles poll/wait, escape quits',
                size: 15, position: [30, 40]
              ))
  window.display!
end

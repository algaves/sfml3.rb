# frozen_string_literal: true

# Rubyesque (Matz-like): the deprecation path. Every name that shipped before the Rubyesque layer
# still exists: it warns once (with the line that called it) and then does
# exactly what it always did. Run this to see the mapping in one place. It
# prints to the console and exits; the window methods need a display.
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/rubyesque/deprecations.rb

require 'sfml'
require_relative '../support'
include SFML

beep = ExampleSupport.sound('beep')
window = Window.new(VideoMode.new(480, 300, 32), 'SFML rubyesque: deprecations')

puts 'Each call below uses a pre-Rubyesque name; the warning names its replacement.'

window.is_open?             # -> open?
window.focus?               # -> focused?
window.request_focus        # -> request_focus!
window.clear                # -> clear!
window.display              # -> display!
beep.play                   # -> play!
beep.pause                  # -> pause!
beep.stop                   # -> stop!
Keyboard.pressed?(:escape)  # -> key_pressed?
Joystick.has_axis?(0, :x)   # -> axis?
Clipboard.string            # -> content
SFML.sleep(SFML::Time.zero) # -> sleep!

window.close!
puts 'All of them still work. Use the banged/predicate names in new code.'

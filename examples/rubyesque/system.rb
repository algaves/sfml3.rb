# frozen_string_literal: true

# Rubyesque (Matz-like): the system and window-adjacent odds and ends that need no window --
# `Clock.measure` times a block, `SFML.sleep!` and `SFML::Sleep.sleep!` pause,
# and the `Clipboard.content` pair gained `has_text?` and `clear!`. Prints to
# the console and exits; the clipboard needs a display, so run it under
# `xvfb-run -a` on a headless machine.
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/rubyesque/system.rb

require 'sfml'
include SFML

# `Clock.measure` makes a throwaway Clock, runs the block and returns the
# elapsed Time. Nothing to construct, restart or read afterwards.
elapsed = Clock.measure do
  SFML.sleep!(SFML::Time.seconds(0.1))
end
puts format('Clock.measure { SFML.sleep!(0.1) } -> %.3fs', elapsed.as_seconds)

# `SFML::Sleep.sleep!` is the namespace-style spelling of the same call.
SFML::Sleep.sleep!(SFML::Time.milliseconds(50))

clock = Clock.new
puts "clock.running?  #{clock.running?}"
clock.stop!
puts "after stop!     #{clock.running?}"
clock.start!
puts "after start!    #{clock.running?}"
puts "restart!        #{clock.restart!.as_seconds.round(3)}s elapsed since the last reset"

# `Clipboard.content` replaces the old `string`/`string=`, and the Rubyesque layer adds
# the two questions CSFML leaves open: is there anything there, and how do I
# empty it.
Clipboard.content = 'sfml3.rb rubyesque'
puts "content=        #{Clipboard.content.inspect}"
puts "has_text?       #{Clipboard.has_text?}"
Clipboard.clear!
puts "after clear!    #{Clipboard.content.inspect}, has_text? #{Clipboard.has_text?}"

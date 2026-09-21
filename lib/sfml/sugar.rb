# frozen_string_literal: true

# Idiomatic, Ruby-flavoured sugar over the native SFML bindings. Every method
# here is defined in pure Ruby on top of the C extension, so the native layer
# stays a thin 1:1 mapping of CSFML while Ruby callers get predicate methods
# (`?`), mutating methods (`!`), block iterators and scoped resources.
#
# The primary names follow the conventions in issue #23. The names that shipped
# before it stay available as deprecated aliases that warn on use.
module SFML
  # Predicate `?` and mutator `!` sugar shared by the whole library.
  #
  # `SFML.sleep` gained a banged primary name so it reads like the rest of the
  # action surface; the old name warns and delegates.
  class << self
    alias sleep! sleep

    def sleep(duration)
      warn 'SFML.sleep is deprecated; use SFML.sleep!', uplevel: 1
      sleep!(duration)
    end
  end

  # `SFML::Sleep.sleep!` mirrors the issue's namespace-style spelling and simply
  # delegates to `SFML.sleep!`.
  module Sleep
    def self.sleep!(duration)
      SFML.sleep!(duration)
    end
  end

  # Tracks the value last passed to `#visible=` so `#visible?` can answer, since
  # CSFML 3 exposes no window-visibility getter. Prepended to each window class
  # that defines its own native `#visible=`.
  module VisibilityTracking
    def visible=(value)
      @sfml_visible = value ? true : false
      super
    end

    def visible?
      @sfml_visible.nil? || @sfml_visible
    end
  end

  # Window sugar shared by every window class: idiomatic aliases, the
  # `poll_events!` block iterator and the scoped `open` constructor.
  #
  # Window redefines every shared method with its `sfRenderWindow` entry point,
  # so the aliases below are repeated there with the correct dispatch.
  class WindowBase
    alias open? is_open?

    def is_open?
      warn "#{self.class}#is_open? is deprecated; use #open?", uplevel: 1
      open?
    end

    alias focused? focus?

    def focus?
      warn "#{self.class}#focus? is deprecated; use #focused?", uplevel: 1
      focused?
    end

    alias request_focus! request_focus

    def request_focus
      warn "#{self.class}#request_focus is deprecated; use #request_focus!", uplevel: 1
      request_focus!
    end

    # Pops every pending event and yields it, returning self. Without a block it
    # returns an Enumerator, so `window.poll_events!.take(5)` works too.
    def poll_events!
      return enum_for(:poll_events!) unless block_given?

      event = Event.new
      yield event while poll_event!(event)

      self
    end

    # Creates a window, yields it, and closes it when the block returns (or
    # raises). Without a block, returns the open window for the caller to close.
    def self.open(video_mode, title, style = :default, state = :windowed)
      window = new(video_mode, title, style, state)
      return window unless block_given?

      begin
        yield window
      ensure
        window.close!
      end
    end
  end

  # The renderable window's sugar: banged frame methods and the scoped
  # `render!` helper. The shared aliases are repeated here because Window
  # overrides the WindowBase entry points with `sfRenderWindow` ones.
  class Window
    alias open? is_open?

    def is_open?
      warn "#{self.class}#is_open? is deprecated; use #open?", uplevel: 1
      open?
    end

    alias focused? focus?

    def focus?
      warn "#{self.class}#focus? is deprecated; use #focused?", uplevel: 1
      focused?
    end

    alias request_focus! request_focus

    def request_focus
      warn "#{self.class}#request_focus is deprecated; use #request_focus!", uplevel: 1
      request_focus!
    end

    alias clear! clear

    def clear(*args)
      warn "#{self.class}#clear is deprecated; use #clear!", uplevel: 1
      clear!(*args)
    end

    alias display! display

    def display
      warn "#{self.class}#display is deprecated; use #display!", uplevel: 1
      display!
    end

    # Clears, yields self for drawing, then presents: the whole frame in one
    # call. Returns self.
    def render!(clear_color: Color::BLACK)
      clear!(clear_color)
      yield self if block_given?
      display!

      self
    end
  end

  WindowBase.prepend(VisibilityTracking)
  Window.prepend(VisibilityTracking)

  # Predicate sugar shared by every playable source. `status` dispatches to the
  # concrete class's native entry point, so these work for Sound, SoundStream
  # and Music alike.
  class SoundSource
    def playing?
      status == :playing
    end

    def paused?
      status == :paused
    end

    def stopped?
      status == :stopped
    end
  end

  # Banged playback mutators for Sound. Defined here rather than on SoundSource
  # because each class owns its own native play/pause/stop.
  class Sound
    alias play! play

    def play
      warn "#{self.class}#play is deprecated; use #play!", uplevel: 1
      play!
    end

    alias pause! pause

    def pause
      warn "#{self.class}#pause is deprecated; use #pause!", uplevel: 1
      pause!
    end

    alias stop! stop

    def stop
      warn "#{self.class}#stop is deprecated; use #stop!", uplevel: 1
      stop!
    end
  end

  # Banged playback mutators for SoundStream (see Sound).
  class SoundStream
    alias play! play

    def play
      warn "#{self.class}#play is deprecated; use #play!", uplevel: 1
      play!
    end

    alias pause! pause

    def pause
      warn "#{self.class}#pause is deprecated; use #pause!", uplevel: 1
      pause!
    end

    alias stop! stop

    def stop
      warn "#{self.class}#stop is deprecated; use #stop!", uplevel: 1
      stop!
    end
  end

  # Banged playback mutators for Music (see Sound).
  class Music
    alias play! play

    def play
      warn "#{self.class}#play is deprecated; use #play!", uplevel: 1
      play!
    end

    alias pause! pause

    def pause
      warn "#{self.class}#pause is deprecated; use #pause!", uplevel: 1
      pause!
    end

    alias stop! stop

    def stop
      warn "#{self.class}#stop is deprecated; use #stop!", uplevel: 1
      stop!
    end
  end

  # Scoped capture: records for the duration of the block and returns the
  # resulting SoundBuffer.
  class SoundBufferRecorder
    def self.record!(sample_rate: 44_100, device: nil, channel_count: nil)
      recorder = new
      recorder.device = device if device
      recorder.channel_count = channel_count if channel_count
      recorder.start(sample_rate)

      begin
        yield recorder
      ensure
        recorder.stop
      end

      recorder.buffer
    end
  end

  # Sensor sugar: `enable!`/`disable!` read better than a boolean argument.
  module Sensor
    class << self
      def enable!(type)
        set_enabled(type, true)
      end

      def disable!(type)
        set_enabled(type, false)
      end
    end
  end

  # Joystick sugar: `axis?` is the predicate spelling the rest of the library
  # uses; `has_axis?` warns and delegates.
  module Joystick
    class << self
      alias axis? has_axis?

      def has_axis?(joystick, axis)
        warn 'SFML::Joystick.has_axis? is deprecated; use Joystick.axis?', uplevel: 1
        axis?(joystick, axis)
      end
    end
  end

  # Keyboard sugar: `key_pressed?` matches the domain wording; `pressed?` warns
  # and delegates.
  module Keyboard
    class << self
      alias key_pressed? pressed?

      def pressed?(key)
        warn 'SFML::Keyboard.pressed? is deprecated; use Keyboard.key_pressed?', uplevel: 1
        key_pressed?(key)
      end
    end
  end

  # Touch sugar: `position` also accepts `relative_to:`, while the positional
  # window argument keeps working.
  module Touch
    class << self
      alias position_without_relative_to position
      private :position_without_relative_to

      def position(finger, window = nil, relative_to: nil)
        position_without_relative_to(finger, window || relative_to)
      end
    end
  end

  # Clipboard sugar: `content`/`content=` replace the `string` pair, and
  # `has_text?`/`clear!` fill the two gaps CSFML leaves.
  module Clipboard
    class << self
      alias content string
      alias content= string=

      def string
        warn 'SFML::Clipboard.string is deprecated; use Clipboard.content', uplevel: 1
        content
      end

      def string=(value)
        warn 'SFML::Clipboard.string= is deprecated; use Clipboard.content=', uplevel: 1
        self.content = value
      end

      def has_text?
        !content.empty?
      end

      def clear!
        self.content = ''
      end
    end
  end

  # System sugar: `Clock.measure` times a block with a throwaway clock.
  class Clock
    def self.measure
      clock = new
      yield
      clock.elapsed_time
    end
  end

  # Event sugar: predicate helpers for every event kind plus `code`, the key
  # code shortcut for key events.
  class Event
    def code
      key[:code]
    end

    def closed?
      type == 'closed'
    end

    def resized?
      type == 'resized'
    end

    def lost_focus?
      type == 'lost-focus'
    end

    def gained_focus?
      type == 'gained-focus'
    end

    def text_entered?
      type == 'text-entered'
    end

    def key_pressed?
      type == 'key-pressed'
    end

    def key_released?
      type == 'key-released'
    end

    def mouse_wheel_scrolled?
      type == 'mouse-wheel-scrolled'
    end

    def mouse_button_pressed?
      type == 'mouse-button-pressed'
    end

    def mouse_button_released?
      type == 'mouse-button-released'
    end

    def mouse_moved?
      type == 'mouse-moved'
    end

    def mouse_moved_raw?
      type == 'mouse-moved-raw'
    end

    def mouse_entered?
      type == 'mouse-entered'
    end

    def mouse_left?
      type == 'mouse-left'
    end

    def joystick_button_pressed?
      type == 'joystick-button-pressed'
    end

    def joystick_button_released?
      type == 'joystick-button-released'
    end

    def joystick_moved?
      type == 'joystick-moved'
    end

    def joystick_connected?
      type == 'joystick-connected'
    end

    def joystick_disconnected?
      type == 'joystick-disconnected'
    end

    def touch_began?
      type == 'touch-began'
    end

    def touch_moved?
      type == 'touch-moved'
    end

    def touch_ended?
      type == 'touch-ended'
    end

    def sensor_changed?
      type == 'sensor-changed'
    end
  end
end

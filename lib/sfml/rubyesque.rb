# frozen_string_literal: true

# Rubyesque (Matz-like) additions to the native SFML bindings. Every method
# here is defined in pure Ruby on top of the C extension, so the native layer
# stays a thin 1:1 mapping of CSFML while Ruby callers get predicate methods
# (`?`), mutating methods (`!`), block iterators and scoped resources.
#
# The primary names follow the conventions in issue #23. The names that shipped
# before it stay available as deprecated aliases that warn on use.
#
# @see README.md#rubyesque-matz-like-layer the narrative walkthrough
# @see examples/rubyesque the runnable teaching scripts
module SFML
  # Predicate `?` and mutator `!` methods shared by the whole library.
  #
  # `SFML.sleep` gained a banged primary name so it reads like the rest of the
  # action surface; the old name warns and delegates.
  class << self
    # Pauses for +duration+.
    #
    # @return [Time, Numeric] +duration+
    #
    # @example
    #   SFML.sleep!(Time.seconds(0.25))
    alias sleep! sleep

    # @deprecated Use {SFML.sleep!} instead.
    # @param duration [Time, Numeric] how long to sleep
    # @return [Time, Numeric] +duration+
    def sleep(duration)
      warn 'SFML.sleep is deprecated; use SFML.sleep!', uplevel: 1
      sleep!(duration)
    end
  end

  # `SFML::Sleep.sleep!` mirrors the issue's namespace-style spelling and simply
  # delegates to `SFML.sleep!`.
  module Sleep
    # Pauses for +duration+; an alias of {SFML.sleep!}.
    #
    # @param duration [Time, Numeric] how long to sleep
    # @return [Time, Numeric] +duration+
    #
    # @example
    #   SFML::Sleep.sleep!(Time.milliseconds(100))
    def self.sleep!(duration)
      SFML.sleep!(duration)
    end
  end

  # Tracks the value last passed to `#visible=` so `#visible?` can answer, since
  # CSFML 3 exposes no window-visibility getter. Prepended to each window class
  # that defines its own native `#visible=`.
  module VisibilityTracking
    # Sets visibility and remembers it for {#visible?}.
    #
    # @param value [Boolean] whether the window should be visible
    # @return [self]
    def visible=(value)
      @sfml_visible = value ? true : false
      super
    end

    # Returns whether the window was last shown.
    #
    # @return [Boolean] +true+ unless the last +visible=+ hid it
    #
    # @example
    #   window.visible = false
    #   window.visible? # => false
    def visible?
      @sfml_visible.nil? || @sfml_visible
    end
  end

  # Window style flags, mirroring the CSFML `sfStyle` values. OR them together
  # to combine flags; the result is an Integer the window constructors accept in
  # place of the `:default`/`:titlebar` symbols.
  #
  # @example
  #   Window.new(VideoMode[640, 480, 32], 'Title', Style::DEFAULT)
  #   Window.new(VideoMode[640, 480, 32], 'Title', Style::TITLEBAR | Style::RESIZE)
  module Style
    NONE = 0
    TITLEBAR = 1
    RESIZE = 2
    CLOSE = 4
    DEFAULT = TITLEBAR | RESIZE | CLOSE
  end

  # Window state, mirroring the CSFML `sfWindowState` values, for the fourth
  # window-constructor argument.
  #
  # @example
  #   Window.new(VideoMode[640, 480, 32], 'Title', Style::DEFAULT, State::FULLSCREEN)
  module State
    WINDOWED = 0
    FULLSCREEN = 1
  end

  # Rubyesque window methods shared by every window class: the predicate/mutator aliases, the
  # `poll_events!` block iterator and the scoped `open` constructor.
  #
  # Window redefines every shared method with its `sfRenderWindow` entry point,
  # so the aliases below are repeated there with the correct dispatch.
  class WindowBase
    # Returns +true+ while the window is open.
    #
    # @return [Boolean]
    #
    # @example
    #   while window.open?
    #     # ...
    #   end
    alias open? is_open?

    # @deprecated Use {#open?} instead.
    # @return [Boolean]
    def is_open?
      warn "#{self.class}#is_open? is deprecated; use #open?", uplevel: 1
      open?
    end

    # Returns +true+ if the window currently has focus.
    #
    # @return [Boolean]
    alias focused? focus?

    # @deprecated Use {#focused?} instead.
    # @return [Boolean]
    def focus?
      warn "#{self.class}#focus? is deprecated; use #focused?", uplevel: 1
      focused?
    end

    # Asks the window manager to give this window focus.
    #
    # @return [self]
    #
    # @example
    #   window.request_focus!
    alias request_focus! request_focus

    # @deprecated Use {#request_focus!} instead.
    # @return [self]
    def request_focus
      warn "#{self.class}#request_focus is deprecated; use #request_focus!", uplevel: 1
      request_focus!
    end

    # Pops every pending event and yields it, returning self. Without a block it
    # returns an Enumerator, so `window.poll_events!.take(5)` works too.
    #
    # @yield [event] each pending event
    # @yieldparam event [Event] the event to handle
    # @return [self] when given a block
    # @return [Enumerator<Event>] when called without a block
    #
    # @example Handling events
    #   window.poll_events! do |event|
    #     window.close! if event.closed?
    #   end
    #
    # @example Consuming them lazily
    #   window.poll_events!.take(5)
    def poll_events!
      return enum_for(:poll_events!) unless block_given?

      event = Event.new
      yield event while poll_event!(event)

      self
    end

    alias poll_event_without_block! poll_event!
    private :poll_event_without_block!
    alias wait_event_without_block! wait_event!
    private :wait_event_without_block!

    # Runs the block repeatedly while the window is open, then closes it.
    #
    # @yield [window] the window, once per frame
    # @return [self] when given a block
    # @return [Enumerator] without a block
    #
    # @example
    #   window.open! do
    #     window.poll_event! { |event| window.close! if event.closed? }
    #     window.clear!
    #     window.display!
    #   end
    def open!(&block)
      return enum_for(:open!) unless block

      begin
        yield self while open?
      ensure
        close!
      end

      self
    end

    # With a block, drains every pending event and yields each, returning self.
    # Without a block, behaves like the native `poll_event!(event) -> bool`.
    #
    # @yield [event] each pending event
    # @return [self] when given a block
    # @return [Boolean] without a block
    def poll_event!(event = nil, &block)
      return poll_event_without_block!(event) if event && !block
      raise ArgumentError, 'poll_event! expects an event or a block' unless block

      buffer = Event.new
      yield buffer while poll_event_without_block!(buffer)

      self
    end

    # With a block, blocks until one event arrives and yields it, returning
    # self. Without a block, behaves like the native `wait_event!(event) -> bool`.
    #
    # @yield [event] the event that arrived
    # @return [self] when given a block
    # @return [Boolean] without a block
    def wait_event!(event = nil, &block)
      return wait_event_without_block!(event) if event && !block
      raise ArgumentError, 'wait_event! expects an event or a block' unless block

      buffer = Event.new
      wait_event_without_block!(buffer)
      yield buffer

      self
    end

    # Creates a window, yields it, and closes it when the block returns (or
    # raises). Without a block, returns the open window for the caller to close.
    #
    # @param video_mode [VideoMode] size and bit depth
    # @param title [String] the window title
    # @param style [Symbol, Array<Symbol>, Integer] window style
    # @param state [Symbol] window state (:windowed, :fullscreen, ...)
    # @yield [window] the freshly created, open window
    # @return [self] when given a block
    # @return [WindowBase] when called without a block
    #
    # @example
    #   WindowBase.open(VideoMode.new(640, 480, 32), 'Title') do |window|
    #     # window is closed for you when this block returns
    #   end
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

  # The renderable window's Rubyesque methods: banged frame methods and the scoped
  # `render!` helper. The shared aliases are repeated here because Window
  # overrides the WindowBase entry points with `sfRenderWindow` ones.
  class Window
    # Returns +true+ while the window is open.
    #
    # @return [Boolean]
    alias open? is_open?

    # @deprecated Use {#open?} instead.
    # @return [Boolean]
    def is_open?
      warn "#{self.class}#is_open? is deprecated; use #open?", uplevel: 1
      open?
    end

    # Returns +true+ if the window currently has focus.
    #
    # @return [Boolean]
    alias focused? focus?

    # @deprecated Use {#focused?} instead.
    # @return [Boolean]
    def focus?
      warn "#{self.class}#focus? is deprecated; use #focused?", uplevel: 1
      focused?
    end

    # Asks the window manager to give this window focus.
    #
    # @return [self]
    alias request_focus! request_focus

    # @deprecated Use {#request_focus!} instead.
    # @return [self]
    def request_focus
      warn "#{self.class}#request_focus is deprecated; use #request_focus!", uplevel: 1
      request_focus!
    end

    # Clears the window to +color+.
    #
    # @return [self]
    #
    # @example
    #   window.clear!([51, 76, 102, 255])
    alias clear! clear

    # @deprecated Use {#clear!} instead.
    # @param args [Array] the clear colour
    # @return [self]
    def clear(*args)
      warn "#{self.class}#clear is deprecated; use #clear!", uplevel: 1
      clear!(*args)
    end

    # Presents everything drawn since the last clear.
    #
    # @return [self]
    alias display! display

    # @deprecated Use {#display!} instead.
    # @return [self]
    def display
      warn "#{self.class}#display is deprecated; use #display!", uplevel: 1
      display!
    end

    alias poll_event_without_block! poll_event!
    private :poll_event_without_block!
    alias wait_event_without_block! wait_event!
    private :wait_event_without_block!

    # With a block, drains every pending event and yields each, returning self.
    # Without a block, behaves like the native `poll_event!(event) -> bool`.
    #
    # @yield [event] each pending event
    # @return [self] when given a block
    # @return [Boolean] without a block
    def poll_event!(event = nil, &block)
      return poll_event_without_block!(event) if event && !block
      raise ArgumentError, 'poll_event! expects an event or a block' unless block

      buffer = Event.new
      yield buffer while poll_event_without_block!(buffer)

      self
    end

    # With a block, blocks until one event arrives and yields it, returning
    # self. Without a block, behaves like the native `wait_event!(event) -> bool`.
    #
    # @yield [event] the event that arrived
    # @return [self] when given a block
    # @return [Boolean] without a block
    def wait_event!(event = nil, &block)
      return wait_event_without_block!(event) if event && !block
      raise ArgumentError, 'wait_event! expects an event or a block' unless block

      buffer = Event.new
      wait_event_without_block!(buffer)
      yield buffer

      self
    end

    # Clears, yields self for drawing, then presents: the whole frame in one
    # call. Returns self.
    #
    # @param clear_color [Color, Array] the colour to clear to before drawing
    # @yield [window] the window to draw into
    # @return [self]
    #
    # @example
    #   window.render!(clear_color: Color::BLACK) do |target|
    #     target.draw sprite
    #   end
    def render!(clear_color: Color::BLACK)
      clear!(clear_color)
      yield self if block_given?
      display!

      self
    end
  end

  WindowBase.prepend(VisibilityTracking)
  Window.prepend(VisibilityTracking)

  # `Class[...]` constructors: the positional spelling the issue proposes. Each
  # delegates to the existing `new` -- setting the position or size afterwards
  # where the native initializer has no such argument -- so the `.new` forms
  # keep working unchanged.

  class VideoMode
    # Builds a VideoMode from a width, a height and an optional bit depth.
    #
    # @return [VideoMode]
    #
    # @example
    #   VideoMode[640, 480, 32]
    def self.[](width, height, bits = 32)
      new(width, height, bits)
    end
  end

  class Vector2
    # Builds a Vector2 from components, an array or another Vector2.
    #
    # @return [Vector2]
    #
    # @example
    #   Vector2[10, 20]
    def self.[](*args)
      new(*args) # steep:ignore
    end
  end

  class Vector3
    # Builds a Vector3 from components, an array or another Vector3.
    #
    # @return [Vector3]
    #
    # @example
    #   Vector3[1, 2, 3]
    def self.[](*args)
      new(*args) # steep:ignore
    end
  end

  class Color
    # Builds a Color from components, an array or a packed Integer.
    #
    # @return [Color]
    #
    # @example
    #   Color[235, 90, 90]
    def self.[](*args)
      new(*args) # steep:ignore
    end
  end

  class Rect
    # Builds a Rect from a position and a size, or an array.
    #
    # @return [Rect]
    #
    # @example
    #   Rect[0, 0, 100, 60]
    def self.[](*args)
      new(*args) # steep:ignore
    end
  end

  class Time
    # Builds a Time from a number of seconds.
    #
    # @return [Time]
    #
    # @example
    #   Time[0.5]
    def self.[](seconds)
      new(Float(seconds))
    end
  end

  class Text
    # Builds a Text, optionally with a font, a string and a character size.
    #
    # @return [Text]
    #
    # @example
    #   Text[font, 'Hello', 24]
    def self.[](*args)
      new(*args) # steep:ignore
    end
  end

  class Vertex
    # Builds a Vertex, optionally with a position, colour and texture coordinates.
    #
    # @return [Vertex]
    #
    # @example
    #   Vertex[Vector2[10, 20], Color[255, 0, 0]]
    def self.[](*args)
      new(*args) # steep:ignore
    end
  end

  class View
    # Builds a View from a Rect (or [left, top, width, height]) or a centre and a
    # size.
    #
    # @return [View]
    #
    # @example
    #   View[Rect[0, 0, 640, 480]]
    #   View[Vector2[320, 240], Vector2[640, 480]]
    def self.[](*args)
      case args.size
      when 1
        rect = args.first
        rect = Rect[*rect] if rect.is_a?(Array) && rect.size == 4
        raise ArgumentError, 'expected a Rect or [left, top, width, height]' unless rect.is_a?(Rect)

        from_rect(rect)
      when 2
        new.tap do |view|
          view.center = args[0].is_a?(Array) ? Vector2[*args[0]] : args[0]
          view.size = args[1].is_a?(Array) ? Vector2[*args[1]] : args[1]
        end
      else
        raise ArgumentError, "wrong number of arguments (given #{args.size}, expected 1..2)"
      end
    end
  end

  class Sprite
    # Builds a Sprite, optionally with a texture.
    #
    # @return [Sprite]
    def self.[](texture)
      new(texture)
    end
  end

  class Texture
    # Builds a Texture from a size ([width, height] or a Vector2).
    #
    # @return [Texture]
    #
    # @example
    #   Texture[[128, 128]]
    def self.[](size)
      new(Vector2[size])
    end
  end

  class Image
    # Builds an Image from a size ([width, height] or a Vector2).
    #
    # @return [Image]
    def self.[](size)
      new(size)
    end
  end

  class RenderTexture
    # Builds a RenderTexture from a size ([width, height] or a Vector2) and
    # optional context settings.
    #
    # @return [RenderTexture]
    def self.[](size, settings = nil)
      new(Vector2[size], settings)
    end
  end

  class CircleShape
    # Builds a CircleShape from a radius and an optional position.
    #
    # @return [CircleShape]
    #
    # @example
    #   CircleShape[30, [100, 100]]
    def self.[](radius, position = [0, 0])
      new(radius).tap { |shape| shape.position = position }
    end
  end

  class RectangleShape
    # Builds a RectangleShape from a position and a size.
    #
    # @return [RectangleShape]
    #
    # @example
    #   RectangleShape[10, 20, 100, 40]
    def self.[](left, top, width, height)
      new([width, height]).tap { |shape| shape.position = [left, top] }
    end
  end

  class ConvexShape
    # Builds a ConvexShape from its points (each an array or a Vector2).
    #
    # @return [ConvexShape]
    #
    # @example
    #   ConvexShape[[0, 0], [100, 0], [50, 80]]
    def self.[](*points)
      new(points.size).tap do |shape|
        points.each_with_index { |point, index| shape.set_point(index, point) }
      end
    end
  end

  # Rubyesque predicates shared by every playable source. `status` dispatches to the
  # concrete class's native entry point, so these work for Sound, SoundStream
  # and Music alike.
  class SoundSource
    # Returns +true+ while the source is playing.
    #
    # @return [Boolean]
    #
    # @example
    #   pause! if music.playing?
    def playing?
      status == :playing
    end

    # Returns +true+ while the source is paused.
    #
    # @return [Boolean]
    def paused?
      status == :paused
    end

    # Returns +true+ when the source is stopped.
    #
    # @return [Boolean]
    #
    # @example
    #   music.play! if music.stopped?
    def stopped?
      status == :stopped
    end
  end

  # Banged playback mutators for Sound. Defined here rather than on SoundSource
  # because each class owns its own native play/pause/stop.
  class Sound
    # Starts (or resumes) playback.
    #
    # @return [self]
    alias play! play

    # @deprecated Use {#play!} instead.
    # @return [self]
    def play
      warn "#{self.class}#play is deprecated; use #play!", uplevel: 1
      play!
    end

    # Pauses playback.
    #
    # @return [self]
    alias pause! pause

    # @deprecated Use {#pause!} instead.
    # @return [self]
    def pause
      warn "#{self.class}#pause is deprecated; use #pause!", uplevel: 1
      pause!
    end

    # Stops playback.
    #
    # @return [self]
    alias stop! stop

    # @deprecated Use {#stop!} instead.
    # @return [self]
    def stop
      warn "#{self.class}#stop is deprecated; use #stop!", uplevel: 1
      stop!
    end
  end

  # Banged playback mutators for SoundStream (see Sound).
  class SoundStream
    # Starts (or resumes) playback.
    #
    # @return [self]
    alias play! play

    # @deprecated Use {#play!} instead.
    # @return [self]
    def play
      warn "#{self.class}#play is deprecated; use #play!", uplevel: 1
      play!
    end

    # Pauses playback.
    #
    # @return [self]
    alias pause! pause

    # @deprecated Use {#pause!} instead.
    # @return [self]
    def pause
      warn "#{self.class}#pause is deprecated; use #pause!", uplevel: 1
      pause!
    end

    # Stops playback.
    #
    # @return [self]
    alias stop! stop

    # @deprecated Use {#stop!} instead.
    # @return [self]
    def stop
      warn "#{self.class}#stop is deprecated; use #stop!", uplevel: 1
      stop!
    end
  end

  # Banged playback mutators for Music (see Sound).
  class Music
    # Starts (or resumes) playback.
    #
    # @return [self]
    alias play! play

    # @deprecated Use {#play!} instead.
    # @return [self]
    def play
      warn "#{self.class}#play is deprecated; use #play!", uplevel: 1
      play!
    end

    # Pauses playback.
    #
    # @return [self]
    alias pause! pause

    # @deprecated Use {#pause!} instead.
    # @return [self]
    def pause
      warn "#{self.class}#pause is deprecated; use #pause!", uplevel: 1
      pause!
    end

    # Stops playback.
    #
    # @return [self]
    alias stop! stop

    # @deprecated Use {#stop!} instead.
    # @return [self]
    def stop
      warn "#{self.class}#stop is deprecated; use #stop!", uplevel: 1
      stop!
    end
  end

  # Scoped capture: records for the duration of the block and returns the
  # resulting SoundBuffer.
  class SoundBufferRecorder
    # Starts the recorder, yields it, stops it when the block returns (or
    # raises) and returns the captured buffer.
    #
    # @param sample_rate [Integer] the capture sample rate
    # @param device [String, nil] the capture device name; the default if nil
    # @param channel_count [Integer, nil] the number of channels to capture
    # @yield [recorder] the recording recorder
    # @return [SoundBuffer] what was captured during the block
    #
    # @example
    #   buffer = SoundBufferRecorder.record!(sample_rate: 44_100) do
    #     SFML.sleep!(Time.seconds(1))
    #   end
    #   Sound.new(buffer).play!
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

  # Rubyesque sensor methods: `enable!`/`disable!` read better than a boolean argument.
  module Sensor
    class << self
      # Enables a hardware sensor.
      #
      # @param type [Symbol, Integer] the sensor type
      # @return [Boolean] +true+
      #
      # @example
      #   Sensor.enable!(:gyroscope)
      def enable!(type)
        set_enabled(type, true)
      end

      # Disables a hardware sensor.
      #
      # @param type [Symbol, Integer] the sensor type
      # @return [Boolean] +false+
      def disable!(type)
        set_enabled(type, false)
      end
    end
  end

  # Rubyesque joystick methods: `axis?` is the predicate spelling the rest of the library
  # uses; `has_axis?` warns and delegates.
  module Joystick
    class << self
      # Returns +true+ if joystick +joystick+ has the given axis.
      #
      # @return [Boolean]
      #
      # @example
      #   Joystick.axis?(0, :z)
      alias axis? has_axis?

      # @deprecated Use {Joystick.axis?} instead.
      # @param joystick [Integer] the joystick index
      # @param axis [Symbol] the axis name
      # @return [Boolean]
      def has_axis?(joystick, axis)
        warn 'SFML::Joystick.has_axis? is deprecated; use Joystick.axis?', uplevel: 1
        axis?(joystick, axis)
      end
    end
  end

  # Rubyesque keyboard methods: `key_pressed?` matches the domain wording; `pressed?` warns
  # and delegates.
  module Keyboard
    class << self
      # Returns +true+ if the key is currently held down.
      #
      # @return [Boolean]
      #
      # @example
      #   Keyboard.key_pressed?(:space)
      alias key_pressed? pressed?

      # @deprecated Use {Keyboard.key_pressed?} instead.
      # @param key [Symbol, String, Integer] the key name or code
      # @return [Boolean]
      def pressed?(key)
        warn 'SFML::Keyboard.pressed? is deprecated; use Keyboard.key_pressed?', uplevel: 1
        key_pressed?(key)
      end
    end
  end

  # Rubyesque touch method: `position` also accepts `relative_to:`, while the positional
  # window argument keeps working.
  module Touch
    class << self
      alias position_without_relative_to position
      private :position_without_relative_to

      # Returns the position of touch point +finger+.
      #
      # @param finger [Integer] the finger index
      # @param window [WindowBase, nil] a window to report coordinates relative to
      # @param relative_to [WindowBase, nil] the same window, as a keyword
      # @return [Vector2]
      #
      # @example
      #   Touch.position(0, relative_to: window)
      def position(finger, window = nil, relative_to: nil)
        position_without_relative_to(finger, window || relative_to)
      end
    end
  end

  # Rubyesque clipboard methods: `content`/`content=` replace the `string` pair, and
  # `has_text?`/`clear!` fill the two gaps CSFML leaves.
  module Clipboard
    class << self
      # Returns the clipboard text.
      #
      # @return [String]
      #
      # @example
      #   puts Clipboard.content
      alias content string
      alias content= string=

      # @deprecated Use {Clipboard.content} instead.
      # @return [String]
      def string
        warn 'SFML::Clipboard.string is deprecated; use Clipboard.content', uplevel: 1
        content
      end

      # @deprecated Use {Clipboard.content=} instead.
      # @param value [String] the text to copy
      # @return [String] +value+
      def string=(value)
        warn 'SFML::Clipboard.string= is deprecated; use Clipboard.content=', uplevel: 1
        self.content = value
      end

      # Returns +true+ if the clipboard holds any text.
      #
      # @return [Boolean]
      def has_text?
        !content.empty?
      end

      # Empties the clipboard.
      #
      # @return [String] the empty string
      def clear!
        self.content = ''
      end
    end
  end

  # Rubyesque system method: `Clock.measure` times a block with a throwaway clock.
  class Clock
    # Runs the block and returns the time it took.
    #
    # @yield the work to time
    # @return [Time] the elapsed time
    #
    # @example
    #   elapsed = Clock.measure do
    #     SFML.sleep!(Time.seconds(0.5))
    #   end
    #   puts "took #{elapsed.as_seconds}s"
    def self.measure
      clock = new
      yield
      clock.elapsed_time
    end
  end

  # Rubyesque event methods: predicate helpers for every event kind plus `code`, the key
  # code shortcut for key events.
  class Event
    # Returns the key code for key events.
    #
    # @return [Symbol] the same as +key[:code]+
    #
    # @example
    #   window.close! if event.key_pressed? && event.code == :escape
    def code
      key[:code]
    end

    # Returns +true+ for a +'closed'+ event.
    #
    # @return [Boolean]
    def closed?
      type == 'closed'
    end

    # Returns +true+ for a +'resized'+ event.
    #
    # @return [Boolean]
    def resized?
      type == 'resized'
    end

    # Returns +true+ for a +'lost-focus'+ event.
    #
    # @return [Boolean]
    def lost_focus?
      type == 'lost-focus'
    end

    # Returns +true+ for a +'gained-focus'+ event.
    #
    # @return [Boolean]
    def gained_focus?
      type == 'gained-focus'
    end

    # Returns +true+ for a +'text-entered'+ event.
    #
    # @return [Boolean]
    def text_entered?
      type == 'text-entered'
    end

    # Returns +true+ for a +'key-pressed'+ event.
    #
    # @return [Boolean]
    #
    # @example
    #   puts event.code if event.key_pressed?
    def key_pressed?
      type == 'key-pressed'
    end

    # Returns +true+ for a +'key-released'+ event.
    #
    # @return [Boolean]
    def key_released?
      type == 'key-released'
    end

    # Returns +true+ for a +'mouse-wheel-scrolled'+ event.
    #
    # @return [Boolean]
    def mouse_wheel_scrolled?
      type == 'mouse-wheel-scrolled'
    end

    # Returns +true+ for a +'mouse-button-pressed'+ event.
    #
    # @return [Boolean]
    def mouse_button_pressed?
      type == 'mouse-button-pressed'
    end

    # Returns +true+ for a +'mouse-button-released'+ event.
    #
    # @return [Boolean]
    def mouse_button_released?
      type == 'mouse-button-released'
    end

    # Returns +true+ for a +'mouse-moved'+ event.
    #
    # @return [Boolean]
    def mouse_moved?
      type == 'mouse-moved'
    end

    # Returns +true+ for a +'mouse-moved-raw'+ event.
    #
    # @return [Boolean]
    def mouse_moved_raw?
      type == 'mouse-moved-raw'
    end

    # Returns +true+ for a +'mouse-entered'+ event.
    #
    # @return [Boolean]
    def mouse_entered?
      type == 'mouse-entered'
    end

    # Returns +true+ for a +'mouse-left'+ event.
    #
    # @return [Boolean]
    def mouse_left?
      type == 'mouse-left'
    end

    # Returns +true+ for a +'joystick-button-pressed'+ event.
    #
    # @return [Boolean]
    def joystick_button_pressed?
      type == 'joystick-button-pressed'
    end

    # Returns +true+ for a +'joystick-button-released'+ event.
    #
    # @return [Boolean]
    def joystick_button_released?
      type == 'joystick-button-released'
    end

    # Returns +true+ for a +'joystick-moved'+ event.
    #
    # @return [Boolean]
    def joystick_moved?
      type == 'joystick-moved'
    end

    # Returns +true+ for a +'joystick-connected'+ event.
    #
    # @return [Boolean]
    def joystick_connected?
      type == 'joystick-connected'
    end

    # Returns +true+ for a +'joystick-disconnected'+ event.
    #
    # @return [Boolean]
    def joystick_disconnected?
      type == 'joystick-disconnected'
    end

    # Returns +true+ for a +'touch-began'+ event.
    #
    # @return [Boolean]
    def touch_began?
      type == 'touch-began'
    end

    # Returns +true+ for a +'touch-moved'+ event.
    #
    # @return [Boolean]
    def touch_moved?
      type == 'touch-moved'
    end

    # Returns +true+ for a +'touch-ended'+ event.
    #
    # @return [Boolean]
    def touch_ended?
      type == 'touch-ended'
    end

    # Returns +true+ for a +'sensor-changed'+ event.
    #
    # @return [Boolean]
    def sensor_changed?
      type == 'sensor-changed'
    end
  end
end

# frozen_string_literal: true

require_relative 'test_helper'

# The pure-Ruby Rubyesque (Matz-like) layer (lib/sfml/rubyesque.rb): predicate
# (`?`) and mutator (`!`) spellings, block helpers, and the deprecated aliases
# that keep the pre-Rubyesque names working. Hardware-free where possible:
# `allocate` gives window objects whose Ruby-only methods can run without a
# display.
class RubyesqueTest < Minitest::Test
  include SFML
  include SFMLTestHelpers

  def test_window_classes_expose_the_primary_rubyesque_names
    %i[open? focused? visible? request_focus! poll_events!].each do |name|
      assert_includes WindowBase.instance_methods, name, "#{name} missing from WindowBase"
    end

    %i[clear! display! render!].each do |name|
      assert_includes Window.instance_methods, name, "#{name} missing from Window"
    end

    assert_respond_to Window, :open
    assert_respond_to RenderWindow, :open
    assert_includes RenderWindow.instance_methods, :render!
  end

  def test_window_rubyesque_names_win_over_the_deprecated_names
    assert_equal WindowBase, WindowBase.instance_method(:open?).owner
    assert_equal Window, Window.instance_method(:open?).owner
    assert_equal Window, RenderWindow.instance_method(:open?).owner
    assert_equal Window, Window.instance_method(:clear!).owner
  end

  def test_deprecated_window_names_still_exist
    %i[is_open? focus? visible?].each do |name|
      assert_includes WindowBase.instance_methods, name
    end

    %i[is_open? focus? request_focus clear display].each do |name|
      assert_includes Window.instance_methods, name
    end
  end

  def test_visible_defaults_to_true_without_a_display
    assert WindowBase.allocate.visible?
  end

  def test_poll_events_returns_an_enumerator_without_a_block
    assert_kind_of Enumerator, WindowBase.allocate.poll_events!
  end

  def test_playback_predicates_live_on_the_sound_source
    %i[playing? paused? stopped?].each do |name|
      assert_includes SoundSource.instance_methods, name, "#{name} missing from SoundSource"
    end
  end

  def test_banged_playback_is_shared_by_every_source
    [Sound, SoundStream, Music].each do |klass|
      %i[play! pause! stop!].each do |name|
        assert_includes klass.instance_methods, name, "#{name} missing from #{klass}"
      end
    end
  end

  def test_audio_record_scoping_is_available
    assert_respond_to SoundBufferRecorder, :record!
  end

  def test_device_modules_expose_the_rubyesque_names
    assert_respond_to Sensor, :enable!
    assert_respond_to Sensor, :disable!
    assert_respond_to Joystick, :axis?
    assert_respond_to Keyboard, :key_pressed?
    assert_respond_to Sensor, :available?
    assert_respond_to Touch, :down?
  end

  def test_touch_position_accepts_the_relative_to_keyword
    parameters = Touch.method(:position).parameters

    assert_includes parameters, %i[key relative_to]
    assert_includes parameters, %i[opt window]
  end

  def test_clipboard_content_and_helpers
    assert_respond_to Clipboard, :content
    assert_respond_to Clipboard, :content=
    assert_respond_to Clipboard, :has_text?
    assert_respond_to Clipboard, :clear!

    Clipboard.content = 'sfml rubyesque'
    assert_equal 'sfml rubyesque', Clipboard.content
    assert Clipboard.has_text?

    Clipboard.clear!
    refute Clipboard.has_text?
  end

  def test_clock_measure_times_a_block
    elapsed = Clock.measure { SFML.sleep!(0.001) }

    assert_kind_of Time, elapsed
    assert_operator elapsed.as_seconds, :>=, 0.0
  end

  def test_sleep_module_delegates_to_the_top_level
    assert_in_epsilon 0.0, SFML::Sleep.sleep!(0), 0.001
  end

  def test_deprecated_sleep_warns_and_still_works
    assert_output(nil, /SFML\.sleep is deprecated/) do
      assert_in_epsilon 0.0, SFML.sleep(0), 0.001
    end
  end

  def test_deprecated_clipboard_spelling_warns_and_still_works
    assert_output(nil, /Clipboard\.string=/i) do
      Clipboard.string = 'old spelling'
    end

    assert_equal 'old spelling', Clipboard.content
  end

  def test_event_predicates_reflect_the_type
    event = Event.new

    assert event.closed?
    refute event.key_pressed?
    refute event.mouse_moved?
    assert event.respond_to?(:sensor_changed?)
  end

  def test_event_code_is_the_key_code
    event = Event.new

    assert_equal event.key[:code], event.code
  end
end

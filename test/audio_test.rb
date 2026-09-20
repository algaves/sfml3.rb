# frozen_string_literal: true

require_relative 'test_helper'

# Audio inheritance: the SoundSource tree (Sound, SoundStream, Music) and the
# separate SoundRecorder tree (SoundBufferRecorder). Structure only, since
# playing or recording needs an audio device.
class AudioTest < Minitest::Test
  include SFML
  include SFMLTestHelpers

  def test_sound_source_is_the_base_of_the_playable_classes
    assert_operator Sound, :<, SoundSource
    assert_operator SoundStream, :<, SoundSource
    assert_operator Music, :<, SoundStream
    assert_operator Music, :<, SoundSource

    refute_operator SoundStream, :<, Sound
    refute_operator Music, :<, Sound
    refute_operator SoundSource, :<, Sound
    assert_equal Object, SoundSource.superclass
  end

  def test_sound_source_declares_no_methods_of_its_own
    # The surface is generated per concrete subclass from
    # ext/audio/sound_source.inc, because each dispatches to a different CSFML
    # entry point.
    assert_empty SoundSource.instance_methods(false)
  end

  def test_playable_classes_share_the_sound_source_surface
    %i[play pause stop status looping? looping= pitch pitch= pan pan= volume volume=
       spatialization_enabled? spatialization_enabled= position position=
       direction direction= velocity velocity= cone cone= doppler_factor doppler_factor=
       directional_attenuation_factor directional_attenuation_factor=
       relative_to_listener? relative_to_listener= min_distance min_distance=
       max_distance max_distance= min_gain min_gain= max_gain max_gain=
       attenuation attenuation= playing_offset playing_offset= effect_processor=].each do |name|
      [Sound, SoundStream, Music].each do |klass|
        assert_includes klass.instance_methods, name, "#{name} missing from #{klass}"
      end
    end
  end

  def test_sound_buffer_recorder_is_a_sound_recorder
    assert_operator SoundBufferRecorder, :<, SoundRecorder
    assert_equal SoundRecorder, SoundBufferRecorder.superclass
    assert_equal Object, SoundRecorder.superclass
  end

  def test_the_recorder_tree_is_separate_from_the_sound_source_tree
    refute_operator SoundRecorder, :<, SoundSource
    refute_operator SoundSource, :<, SoundRecorder
    refute_operator SoundBufferRecorder, :<, SoundSource
  end

  def test_sound_buffer_recorder_drops_channel_map
    assert_includes SoundRecorder.instance_methods, :channel_map
    refute_includes SoundBufferRecorder.instance_methods, :channel_map
  end
end

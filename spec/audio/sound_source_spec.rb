# frozen_string_literal: true

require_relative '../spec_helper'

# The SoundSource tree: Sound, SoundStream and Music share the source surface
# via ext/audio/sound_source.inc, and the Rubyesque playback predicates live on
# the base. Structure only: playing needs an audio device.
RSpec.describe SF::Audio::SoundSource do
  it 'makes SoundSource the base of the playable classes' do
    expect(SF::Audio::Sound).to be < SF::Audio::SoundSource
    expect(SF::Audio::SoundStream).to be < SF::Audio::SoundSource
    expect(SF::Audio::Music).to be < SF::Audio::SoundStream
    expect(SF::Audio::Music).to be < SF::Audio::SoundSource

    expect(SF::Audio::SoundStream).not_to be < SF::Audio::Sound
    expect(SF::Audio::Music).not_to be < SF::Audio::Sound
    expect(SF::Audio::SoundSource).not_to be < SF::Audio::Sound
    expect(SF::Audio::SoundSource.superclass).to eq(Object)
  end

  it 'carries the shared playback predicates on SoundSource' do
    # The native surface is generated per concrete subclass from
    # ext/audio/sound_source.inc, because each dispatches to a different CSFML
    # entry point. The Rubyesque predicates are the one shared addition: they
    # read #status, which every concrete subclass provides.
    %i[playing? paused? stopped?].each do |name|
      expect(SF::Audio::SoundSource.instance_methods(false)).to include(name)
    end
  end

  it 'puts the playback predicates on SoundSource' do
    %i[playing? paused? stopped?].each do |name|
      expect(SF::Audio::SoundSource.instance_methods).to include(name)
    end
  end

  it 'shares the banged playback between every source' do
    [SF::Audio::Sound, SF::Audio::SoundStream, SF::Audio::Music].each do |klass|
      %i[play! pause! stop!].each do |name|
        expect(klass.instance_methods).to include(name)
      end
    end
  end

  it 'shares the sound source surface between the playable classes' do
    %i[play pause stop status looping? looping= pitch pitch= pan pan= volume volume=
       spatialization_enabled? spatialization_enabled= position position=
       direction direction= velocity velocity= cone cone= doppler_factor doppler_factor=
       directional_attenuation_factor directional_attenuation_factor=
       relative_to_listener? relative_to_listener= min_distance min_distance=
       max_distance max_distance= min_gain min_gain= max_gain max_gain=
       attenuation attenuation= playing_offset playing_offset= effect_processor=].each do |name|
      [SF::Audio::Sound, SF::Audio::SoundStream, SF::Audio::Music].each do |klass|
        expect(klass.instance_methods).to include(name)
      end
    end
  end

  it 'exposes the enum constants' do
    expect(SF::Audio::SoundStatus::STOPPED).to eq(0)
    expect(SF::Audio::SoundStatus::PLAYING).to eq(2)
    expect(SF::Audio::SoundChannel::UNSPECIFIED).to eq(0)
    expect(SF::Audio::SoundChannel::FRONT_LEFT).to be_a(Integer)
  end

  it 'constructs and compares sound source cones' do
    cone = SF::Audio::SoundSourceCone.new(10, 20, 0.5)

    expect(cone.inner_angle).to be_within(0.001).of(10)
    expect(cone.outer_angle).to be_within(0.001).of(20)
    expect(cone.outer_gain).to be_within(0.001).of(0.5)
    expect(cone.to_a).to eq([10, 20, 0.5])
    expect(SF::Audio::SoundSourceCone.new(10, 20, 0.5)).to eq(cone)
  end
end

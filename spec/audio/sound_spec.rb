# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Audio::Sound do
  it 'round-trips sound properties' do
    buffer = SF::Audio::SoundBuffer.from_samples([0, 0, 0, 0], 1, 44_100)
    sound = SF::Audio::Sound.new(buffer)

    sound.volume = 0.25
    sound.pitch = 1.5
    sound.pan = -0.5
    sound.position = [4, 5, 6]
    sound.cone = SF::Audio::SoundSourceCone.new(90, 180, 0.1)

    expect(sound.volume).to be_within(0.001).of(0.25)
    expect(sound.pitch).to be_within(0.001).of(1.5)
    expect(sound.pan).to be_within(0.001).of(-0.5)
    expect(sound.position.to_a).to be_vec_in_epsilon([4, 5, 6])
    expect(sound.status).to eq(:stopped)

    copy = sound.copy
    expect(copy.volume).to be_within(0.001).of(0.25)
  end

  it 'assigns and clears an effect processor' do
    buffer = SF::Audio::SoundBuffer.from_samples([0, 0], 1, 44_100)
    sound = SF::Audio::Sound.new(buffer)

    sound.effect_processor = proc { |frames, _channels| frames }
    sound.effect_processor = nil
    expect(sound).to be_a(SF::Audio::Sound)
  end
end

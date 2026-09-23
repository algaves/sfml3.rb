# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Audio::Listener do
  it 'round-trips the global listener' do
    SF::Audio::Listener.global_volume = 33.0
    expect(SF::Audio::Listener.global_volume).to be_within(0.001).of(33.0)

    SF::Audio::Listener.position = [1, 2, 3]
    expect(SF::Audio::Listener.position.to_a).to be_vec_in_epsilon([1, 2, 3])

    SF::Audio::Listener.up_vector = [0, 1, 0]
    expect(SF::Audio::Listener.up_vector.to_a).to be_vec_in_epsilon([0, 1, 0])

    SF::Audio::Listener.cone = [10, 20, 0.25]
    expect(SF::Audio::Listener.cone.outer_gain).to be_within(0.001).of(0.25)
  ensure
    SF::Audio::Listener.global_volume = 100.0
    SF::Audio::Listener.position = [0, 0, 0]
  end
end

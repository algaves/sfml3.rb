# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Listener do
  it 'round-trips the global listener' do
    SFML::Listener.global_volume = 33.0
    expect(SFML::Listener.global_volume).to be_within(0.001).of(33.0)

    SFML::Listener.position = [1, 2, 3]
    expect(SFML::Listener.position.to_a).to be_vec_in_epsilon([1, 2, 3])

    SFML::Listener.up_vector = [0, 1, 0]
    expect(SFML::Listener.up_vector.to_a).to be_vec_in_epsilon([0, 1, 0])

    SFML::Listener.cone = [10, 20, 0.25]
    expect(SFML::Listener.cone.outer_gain).to be_within(0.001).of(0.25)
  ensure
    SFML::Listener.global_volume = 100.0
    SFML::Listener.position = [0, 0, 0]
  end
end

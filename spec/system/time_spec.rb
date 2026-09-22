# frozen_string_literal: true

require_relative '../spec_helper'

# SFML::Time shadows Ruby's core Time under `include SFML`, so every constant
# below is namespace-qualified rather than resolved lexically.
RSpec.describe SFML::Time do
  it 'constructs from the unit factories' do
    expect(SFML::Time.seconds(1.5).as_seconds).to be_within(0.001).of(1.5)
    expect(SFML::Time.milliseconds(1500).as_milliseconds).to eq(1500)
    expect(SFML::Time.microseconds(2500).as_microseconds).to eq(2500)
    expect(SFML::Time.zero.as_microseconds).to eq(0)
  end

  it 'supports arithmetic and comparison' do
    a = SFML::Time.seconds(2)
    b = SFML::Time.seconds(0.5)

    expect((a + b).as_seconds).to be_within(0.001).of(2.5)
    expect((a - b).as_seconds).to be_within(0.001).of(1.5)
    expect((a * 2).as_seconds).to be_within(0.001).of(4.0)
    expect(a).to be > b
    expect(a).to eq(SFML::Time.seconds(2))
  end
end

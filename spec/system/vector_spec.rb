# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Vector2 do
  it 'constructs and queries' do
    v = SFML::Vector2.new(3, 4)
    expect(v.x).to be_within(0.001).of(3)
    expect(v.y).to be_within(0.001).of(4)
    expect(v.to_a).to eq([3.0, 4.0])
  end

  it 'accepts an array and compares for equality' do
    expect(SFML::Vector2.new([1, 2])).to eq(SFML::Vector2.new(1, 2))
    expect(SFML::Vector2.new(2, 1)).not_to eq(SFML::Vector2.new(1, 2))
    expect(SFML::Vector2.new(1, 2).to_a).to eq([1.0, 2.0])
  end

  it 'supports arithmetic' do
    expect((SFML::Vector2.new(1, 2) + [3, 4]).to_a).to eq([4.0, 6.0])
    expect((SFML::Vector2.new(1, 2) - [3, 4]).to_a).to eq([-2.0, -2.0])
    expect((SFML::Vector2.new(1, 2) * 2).to_a).to eq([2.0, 4.0])
  end
end

RSpec.describe SFML::Vector3 do
  it 'constructs and compares' do
    v = SFML::Vector3.new(1, 2, 3)
    expect(v.x).to be_within(0.001).of(1)
    expect(v.z).to be_within(0.001).of(3)
    expect(v.to_a).to eq([1.0, 2.0, 3.0])
    expect(SFML::Vector3.new([1, 2, 3])).to eq(v)
  end
end

# Bracket constructors span both vector sizes, so they stay as one cartesian
# block rather than bespoke on each vector class.
RSpec.describe 'Bracket constructors for vectors' do
  it 'build vectors' do
    expect(SFML::Vector2[1, 2].to_a).to eq([1.0, 2.0])
    expect(SFML::Vector2[[3, 4]].to_a).to eq([3.0, 4.0])
    expect(SFML::Vector3[1, 2, 3].to_a).to eq([1.0, 2.0, 3.0])
    expect(SFML::Vector3[[4, 5, 6]].to_a).to eq([4.0, 5.0, 6.0])
  end
end

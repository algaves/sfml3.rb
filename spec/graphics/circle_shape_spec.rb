# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::CircleShape do
  it 'aliases Circle to the canonical CircleShape' do
    expect(SFML::Circle).to be(SFML::CircleShape)
    expect(SFML::Circle.new(1)).to be_a(SFML::CircleShape)
    expect(SFML::CircleShape.name).to eq('SFML::CircleShape')
  end

  it 'defaults to a zero radius' do
    expect(SFML::Circle.new.radius).to be_within(0.01).of(0)
  end

  it 'keeps the radius passed to the constructor' do
    expect(SFML::Circle.new(10).radius).to be_within(0.01).of(10)
  end

  it 'rejects too many arguments' do
    expect { SFML::Circle.new(1, 2) }.to raise_error(ArgumentError)
  end

  it 'sets and returns the point count' do
    c = SFML::Circle.new(10)
    c.point_count = 8
    expect(c.point_count).to eq(8)
  end
end

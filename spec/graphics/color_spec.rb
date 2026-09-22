# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Color do
  it 'constructs from components' do
    c = SFML::Color.new(10, 20, 30, 40)
    expect(c.r).to eq(10)
    expect(c.a).to eq(40)
    expect(c.to_a).to eq([10, 20, 30, 40])
  end

  it 'accepts an array and an integer' do
    expect(SFML::Color.new([10, 20, 30, 255])).to eq(SFML::Color.new(10, 20, 30))
    expect(SFML::Color.new(0x11223344)).to eq(SFML::Color.new(0x11, 0x22, 0x33, 0x44))
    expect(SFML::Color.new(0x11223344).to_i).to eq(0x11223344)
  end

  it 'exposes the predefined constants and operations' do
    expect(SFML::Color::WHITE.to_a).to eq([255, 255, 255, 255])
    expect(SFML::Color::BLACK.to_a).to eq([0, 0, 0, 255])
    expect((SFML::Color.new(100, 100, 100) + SFML::Color.new(200, 200, 200)).to_a)
      .to eq([255, 255, 255, 255])
  end

  # The Rubyesque bracket constructor sits with Color even though it also covers
  # Rect and Time, so the single example block stays intact.
  describe 'Bracket constructor' do
    it 'build Color, Rect and Time' do
      expect(SFML::Color[1, 2, 3].to_a).to eq([1, 2, 3, 255])
      expect(SFML::Color[0x11223344]).to eq(SFML::Color.from_integer(0x11223344))
      expect(SFML::Rect[0, 0, 10, 20].to_a).to eq([0.0, 0.0, 10.0, 20.0])
      expect(SFML::Rect[1, 2].to_a).to eq([1.0, 2.0, 0.0, 0.0])

      expect(SFML::Time[0.5].as_seconds).to be_within(1e-9).of(0.5)
      expect(SFML::Time[2].as_seconds).to be_within(1e-9).of(2.0)
    end
  end
end

# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Graphics::Color do
  it 'constructs from components' do
    c = SF::Graphics::Color.new(10, 20, 30, 40)
    expect(c.r).to eq(10)
    expect(c.a).to eq(40)
    expect(c.to_a).to eq([10, 20, 30, 40])
  end

  it 'accepts an array and an integer' do
    expect(SF::Graphics::Color.new([10, 20, 30, 255])).to eq(SF::Graphics::Color.new(10, 20, 30))
    expect(SF::Graphics::Color.new(0x11223344)).to eq(SF::Graphics::Color.new(0x11, 0x22, 0x33, 0x44))
    expect(SF::Graphics::Color.new(0x11223344).to_i).to eq(0x11223344)
  end

  it 'exposes the predefined constants and operations' do
    expect(SF::Graphics::Color::WHITE.to_a).to eq([255, 255, 255, 255])
    expect(SF::Graphics::Color::BLACK.to_a).to eq([0, 0, 0, 255])
    expect((SF::Graphics::Color.new(100, 100, 100) + SF::Graphics::Color.new(200, 200, 200)).to_a)
      .to eq([255, 255, 255, 255])
  end

  # The Rubyesque bracket constructor sits with Color even though it also covers
  # Rect and Time, so the single example block stays intact.
  describe 'Bracket constructor' do
    it 'build Color, Rect and Time' do
      expect(SF::Graphics::Color[1, 2, 3].to_a).to eq([1, 2, 3, 255])
      expect(SF::Graphics::Color[0x11223344]).to eq(SF::Graphics::Color.from_integer(0x11223344))
      expect(SF::Graphics::Rect[0, 0, 10, 20].to_a).to eq([0.0, 0.0, 10.0, 20.0])
      expect(SF::Graphics::Rect[1, 2].to_a).to eq([1.0, 2.0, 0.0, 0.0])

      expect(SF::System::Time[0.5].as_seconds).to be_within(1e-9).of(0.5)
      expect(SF::System::Time[2].as_seconds).to be_within(1e-9).of(2.0)
    end
  end
end

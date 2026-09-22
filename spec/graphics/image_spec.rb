# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Image do
  it 'constructs from a color and reads pixels' do
    image = SFML::Image.from_color([4, 4], SFML::Color.new(10, 20, 30, 255))
    expect(image.size.to_a).to eq([4.0, 4.0])
    expect(image.pixel(1, 1).to_a).to eq([10, 20, 30, 255])
    expect(image.pixels.bytesize).to eq(4 * 4 * 4)
  end

  it 'sets pixels and copies' do
    image = SFML::Image.new([2, 2])
    image.set_pixel(0, 0, SFML::Color::RED)
    expect(image.pixel(0, 0).to_a).to eq([255, 0, 0, 255])

    copy = image.copy
    expect(copy.pixel(0, 0)).to eq(image.pixel(0, 0))
  end

  it 'constructs from packed pixels' do
    pixels = [255, 0, 0, 255, 0, 255, 0, 255].pack('C*')
    image = SFML::Image.from_pixels([2, 1], pixels)
    expect(image.pixel(0, 0).to_a).to eq([255, 0, 0, 255])
    expect(image.pixel(1, 0).to_a).to eq([0, 255, 0, 255])
  end

  it 'saves to memory' do
    image = SFML::Image.from_color([2, 2], SFML::Color::WHITE)
    buffer = image.save_to_memory
    expect(buffer).to be_a(SFML::Buffer)
    expect(buffer).not_to be_empty
  end
end

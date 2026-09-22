# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Rect do
  it 'constructs and queries geometry' do
    r = SFML::Rect.new(1, 2, 10, 20)
    expect(r.left).to be_within(0.001).of(1)
    expect(r.height).to be_within(0.001).of(20)
    expect(r.to_a).to eq([1.0, 2.0, 10.0, 20.0])
    expect(r.position).to eq(SFML::Vector2.new(1, 2))
    expect(r.size).to eq(SFML::Vector2.new(10, 20))
  end

  it 'tests containment and intersection' do
    r = SFML::Rect.new(0, 0, 10, 10)
    expect(r).to be_contains([5, 5])
    expect(r).not_to be_contains([15, 5])

    expect(r).to be_intersects(SFML::Rect.new(5, 5, 10, 10))
    expect(r).not_to be_intersects(SFML::Rect.new(20, 20, 5, 5))

    intersection = r.intersection(SFML::Rect.new(5, 5, 10, 10))
    expect(intersection.to_a).to eq([5.0, 5.0, 5.0, 5.0])
    expect(r.intersection(SFML::Rect.new(20, 20, 5, 5))).to be_nil
  end
end

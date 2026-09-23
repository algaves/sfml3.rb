# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Graphics::RectangleShape do
  it 'constructs a rectangle shape' do
    shape = SF::Graphics::RectangleShape.new([10, 20])
    expect(shape.size).to be_vec_in_epsilon([10, 20])
    expect(shape.point_count).to eq(4)
    expect(shape.local_bounds.size.to_a.reduce(:*)).to be_within(0.01).of(200)
  end
end

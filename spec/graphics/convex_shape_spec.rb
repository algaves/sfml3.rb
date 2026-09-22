# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Graphics::ConvexShape do
  it 'sets points on a convex shape' do
    shape = SF::Graphics::ConvexShape.new(3)
    shape.set_point 0, [0, 0]
    shape.set_point 1, [10, 0]
    shape.set_point 2, [0, 10]
    expect(shape.point_count).to eq(3)
    expect(shape.point(1)).to be_vec_in_epsilon([10, 0])
  end
end

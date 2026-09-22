# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::RenderState do
  it 'defaults to a 9-element transform matrix' do
    matrix = SFML::RenderState.new.transform
    expect(matrix).to be_a(Array)
    expect(matrix.length).to eq(9)
  end

  it 'round-trips a matrix' do
    rs = SFML::RenderState.new
    rs.transform = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    expect(rs.transform).to be_vec_in_epsilon([1, 0, 0, 0, 1, 0, 0, 0, 1])
    expect(rs.matrix).to eq(rs.transform)
  end

  it 'accepts a Transform or an Array' do
    rs = SFML::RenderState.new
    t = SFML::Transform.identity.translate!([3, 4])

    rs.transform = t
    expect(rs.transform).to be_matrix_in_delta(t.to_a)

    rs.transform = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    expect(rs.transform).to be_matrix_in_delta([1, 0, 0, 0, 1, 0, 0, 0, 1])
  end

  it 'carries blend, stencil and coordinate state' do
    rs = SFML::RenderState.new
    rs.blend_mode = SFML::BlendMode::NONE
    expect(rs.blend_mode.color_src_factor).to eq(:one)

    rs.stencil_mode = SFML::StencilMode.new(:always, :keep, 0, 0, false)
    expect(rs.stencil_mode.comparison).to eq(:always)

    rs.coordinate_type = :pixels
    expect(rs.coordinate_type).to eq(:pixels)
  end
end

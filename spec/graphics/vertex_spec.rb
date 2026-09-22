# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::VertexArray do
  it 'constructs a vertex' do
    vertex = SFML::Vertex.new([1, 2], SFML::Color::WHITE, [0.5, 0.25])
    expect(vertex.position).to be_vec_in_epsilon([1, 2])
    expect(vertex.color.to_a).to eq([255, 255, 255, 255])
    expect(vertex.tex_coords).to be_vec_in_epsilon([0.5, 0.25])
  end

  it 'appends and queries vertices' do
    array = SFML::VertexArray.new
    array.append(SFML::Vertex.new([0, 0]))
    array.append([10, 0])
    array.primitive = :triangles

    expect(array.vertex_count).to eq(2)
    expect(array.primitive).to eq(:triangles)
    expect(array.vertex(1).position.to_a).to eq([10.0, 0.0])
  end
end

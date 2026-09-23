# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Graphics::Transform do
  it 'exposes an identity' do
    expect(SF::Graphics::Transform.identity.to_a).to eq([1, 0, 0, 0, 1, 0, 0, 0, 1])
    expect(SF::Graphics::Transform::IDENTITY).to eq(SF::Graphics::Transform.identity)
  end

  it 'freezes the shared IDENTITY constant' do
    # A shared constant that mutators could edit would silently corrupt every
    # later use of it.
    expect(SF::Graphics::Transform::IDENTITY).to be_frozen
    expect { SF::Graphics::Transform::IDENTITY.translate!([1, 1]) }.to raise_error(FrozenError)
    expect(SF::Graphics::Transform::IDENTITY.to_a).to eq([1, 0, 0, 0, 1, 0, 0, 0, 1])
  end

  it 'round-trips through from_a and new' do
    t = SF::Graphics::Transform.identity.translate!([4, 9]).rotate!(30)
    expect(SF::Graphics::Transform.from_a(t.to_a)).to eq(t)
    expect(SF::Graphics::Transform.new(*t.to_a).to_a).to eq(t.to_a)
  end

  it 'translates points' do
    t = SF::Graphics::Transform.identity.translate!([10, 20])
    expect(t.to_a).to be_matrix_in_delta([1, 0, 10, 0, 1, 20, 0, 0, 1])
    expect(t.transform_point([1, 2])).to be_vec_in_epsilon([11, 22])
  end

  it 'scales about a center point' do
    # Scaling about (1, 1) leaves that point where it is.
    t = SF::Graphics::Transform.identity.scale!([2, 3], [1, 1])
    expect(t.transform_point([1, 1])).to be_vec_in_epsilon([1, 1])
    expect(t.transform_point([2, 2])).to be_vec_in_epsilon([3, 4])
  end

  it 'rotates a point a quarter turn' do
    point = SF::Graphics::Transform.identity.rotate!(90).transform_point([1, 0])
    expect(point.x).to be_within(0.0001).of(0)
    expect(point.y).to be_within(0.0001).of(1)
  end

  it 'chains mutators returning self' do
    t = SF::Graphics::Transform.identity
    expect(t.translate!([1, 1])).to be(t)
    expect(t.rotate!(10)).to be(t)
    expect(t.scale!([2, 2])).to be(t)
  end

  it 'leaves the receiver unmutated on the non-bang forms' do
    t = SF::Graphics::Transform.identity
    moved = t.translate([5, 5])
    expect(t).to eq(SF::Graphics::Transform.identity)
    expect(moved).not_to eq(t)
  end

  it 'cancels itself against its inverse' do
    t = SF::Graphics::Transform.identity.translate!([7, -3]).rotate!(25).scale!([2, 4])
    expect((t * t.inverse).to_a).to be_matrix_in_delta(SF::Graphics::Transform.identity.to_a)
  end

  it 'transforms a rect' do
    rect = SF::Graphics::Transform.identity.translate!([10, 10]).transform_rect([0, 0, 4, 6])
    expect(rect.to_a).to be_matrix_in_delta([10, 10, 4, 6])
  end

  it 'exposes a 4x4 GL matrix distinct from to_a' do
    # Distinct from #to_a: sfTransform_getMatrix fills the 16-float OpenGL form.
    matrix = SF::Graphics::Transform.identity.gl_matrix
    expect(matrix.length).to eq(16)
    expect(SF::Graphics::Transform.identity.to_a.length).to eq(9)
  end

  it 'keeps the module functions Array-in / Array-out' do
    # Transform was a module before it was a class; both entry points stay
    # Array-in/Array-out so existing callers keep working.
    identity = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    moved = [1, 0, 5, 0, 1, 0, 0, 0, 1]

    combined = SF::Graphics::Transform.combine(identity, moved)
    expect(combined).to be_a(Array)
    expect(combined).to be_matrix_in_delta(moved)
    expect(SF::Graphics::Transform.inverse(identity)).to be_matrix_in_delta(identity)
  end
end

# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Graphics::View do
  it 'derives center and size from a rect' do
    # The rect is the visible area, so its centre is the view's centre.
    view = SF::Graphics::View.from_rect [0, 0, 100, 50]
    expect(view.center).to be_vec_in_epsilon([50, 25])
    expect(view.size).to be_vec_in_epsilon([100, 50])
  end

  it 'round-trips its scissor rect' do
    view = SF::Graphics::View.new
    expect(view.scissor.to_a).to be_vec_in_epsilon([0, 0, 1, 1])

    view.scissor = [0.25, 0.25, 0.5, 0.5]
    expect(view.scissor.to_a).to be_vec_in_epsilon([0.25, 0.25, 0.5, 0.5])
  end

  it 'builds from bracket constructors' do
    expect(SF::Graphics::View[SF::Graphics::Rect[0, 0, 10, 20]].size.to_a).to be_vec_in_epsilon([10, 20])
    expect(SF::Graphics::View[[0, 0, 10, 20]].center.to_a).to be_vec_in_epsilon([5, 10])

    expect(SF::Graphics::View[[5, 6], [10, 20]].center.to_a).to be_vec_in_epsilon([5, 6])
    expect(SF::Graphics::View[[5, 6], [10, 20]].size.to_a).to be_vec_in_epsilon([10, 20])

    expect { SF::Graphics::View[1, 2, 3] }.to raise_error(ArgumentError)
    expect { SF::Graphics::View[[1, 2]] }.to raise_error(ArgumentError)
  end
end

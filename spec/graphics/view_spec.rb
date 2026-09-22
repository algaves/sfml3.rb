# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::View do
  it 'derives center and size from a rect' do
    # The rect is the visible area, so its centre is the view's centre.
    view = SFML::View.from_rect [0, 0, 100, 50]
    expect(view.center).to be_vec_in_epsilon([50, 25])
    expect(view.size).to be_vec_in_epsilon([100, 50])
  end

  it 'round-trips its scissor rect' do
    view = SFML::View.new
    expect(view.scissor.to_a).to be_vec_in_epsilon([0, 0, 1, 1])

    view.scissor = [0.25, 0.25, 0.5, 0.5]
    expect(view.scissor.to_a).to be_vec_in_epsilon([0.25, 0.25, 0.5, 0.5])
  end

  it 'builds from bracket constructors' do
    expect(SFML::View[SFML::Rect[0, 0, 10, 20]].size.to_a).to be_vec_in_epsilon([10, 20])
    expect(SFML::View[[0, 0, 10, 20]].center.to_a).to be_vec_in_epsilon([5, 10])

    expect(SFML::View[[5, 6], [10, 20]].center.to_a).to be_vec_in_epsilon([5, 6])
    expect(SFML::View[[5, 6], [10, 20]].size.to_a).to be_vec_in_epsilon([10, 20])

    expect { SFML::View[1, 2, 3] }.to raise_error(ArgumentError)
    expect { SFML::View[[1, 2]] }.to raise_error(ArgumentError)
  end
end

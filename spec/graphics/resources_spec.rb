# frozen_string_literal: true

require_relative '../spec_helper'

# The Rubyesque resource bracket constructors: Sprite, Texture, Image,
# RenderTexture and Text all respond to #[] (their arguments feed the
# underlying sf*_create/load-from-disk path).
RSpec.describe 'resource bracket constructors' do
  it 'declares the resource bracket constructors' do
    [SFML::Sprite, SFML::Texture, SFML::Image, SFML::RenderTexture, SFML::Text].each do |klass|
      expect(klass).to respond_to(:[])
    end

    expect(SFML::Image[[4, 4]].size.to_a).to be_vec_in_epsilon([4, 4])
  end
end

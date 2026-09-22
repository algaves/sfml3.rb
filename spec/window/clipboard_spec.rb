# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Clipboard do
  it 'round-trips the clipboard' do
    SFML::Clipboard.string = 'sfml clipboard'
    expect(SFML::Clipboard.string).to eq('sfml clipboard')
  end

  it 'exposes the content and helper methods' do
    expect(SFML::Clipboard).to respond_to(:content)
    expect(SFML::Clipboard).to respond_to(:content=)
    expect(SFML::Clipboard).to respond_to(:has_text?)
    expect(SFML::Clipboard).to respond_to(:clear!)

    SFML::Clipboard.content = 'sfml rubyesque'
    expect(SFML::Clipboard.content).to eq('sfml rubyesque')
    expect(SFML::Clipboard).to have_text

    SFML::Clipboard.clear!
    expect(SFML::Clipboard).not_to have_text
  end
end

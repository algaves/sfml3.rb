# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::Clipboard do
  it 'round-trips the clipboard' do
    SF::Window::Clipboard.string = 'sfml clipboard'
    expect(SF::Window::Clipboard.string).to eq('sfml clipboard')
  end

  it 'exposes the content and helper methods' do
    expect(SF::Window::Clipboard).to respond_to(:content)
    expect(SF::Window::Clipboard).to respond_to(:content=)
    expect(SF::Window::Clipboard).to respond_to(:has_text?)
    expect(SF::Window::Clipboard).to respond_to(:clear!)

    SF::Window::Clipboard.content = 'sfml rubyesque'
    expect(SF::Window::Clipboard.content).to eq('sfml rubyesque')
    expect(SF::Window::Clipboard).to have_text

    SF::Window::Clipboard.clear!
    expect(SF::Window::Clipboard).not_to have_text
  end
end

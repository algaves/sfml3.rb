# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::Keyboard do
  it 'maps between keys and scancodes' do
    expect(SF::Window::Keyboard.delocalize(:a)).to eq(0)
    expect(SF::Window::Keyboard.localize(4)).to eq(:e)
    expect(SF::Window::Keyboard.delocalize('a')).to eq(0)
  end

  it 'rejects unknown keys' do
    expect { SF::Window::Keyboard.delocalize(:not_a_key) }.to raise_error(ArgumentError)
  end
end

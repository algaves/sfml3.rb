# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::Cursor do
  it 'creates a system cursor' do
    expect(SF::Window::Cursor.from_system(:hand)).to be_a(SF::Window::Cursor)
  end
end

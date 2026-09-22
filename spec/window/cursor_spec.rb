# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Cursor do
  it 'creates a system cursor' do
    expect(SFML::Cursor.from_system(:hand)).to be_a(SFML::Cursor)
  end
end

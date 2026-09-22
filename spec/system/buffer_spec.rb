# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Buffer do
  it 'starts empty' do
    buffer = SFML::Buffer.new
    expect(buffer).to be_empty
    expect(buffer.size).to eq(0)
    expect(buffer.data).to eq('')
  end
end

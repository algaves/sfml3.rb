# frozen_string_literal: true

require_relative '../spec_helper'
require 'stringio'

RSpec.describe SFML::InputStream do
  it 'wraps an IO' do
    io = StringIO.new('hello')
    stream = SFML::InputStream.new(io)
    expect(stream.io).to be(io)
  end

  it 'requires a readable object' do
    expect { SFML::InputStream.new(Object.new) }.to raise_error(ArgumentError)
  end
end

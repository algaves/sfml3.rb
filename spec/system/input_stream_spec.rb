# frozen_string_literal: true

require_relative '../spec_helper'
require 'stringio'

RSpec.describe SF::System::InputStream do
  it 'wraps an IO' do
    io = StringIO.new('hello')
    stream = SF::System::InputStream.new(io)
    expect(stream.io).to be(io)
  end

  it 'requires a readable object' do
    expect { SF::System::InputStream.new(Object.new) }.to raise_error(ArgumentError)
  end
end

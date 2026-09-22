# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::SoundStream do
  it 'drives a SoundStream subclass' do
    klass = Class.new(SFML::SoundStream) do
      def on_get_data
        [0, 0]
      end
    end

    stream = klass.new(1, 44_100, [:mono])

    expect(stream.channel_count).to eq(1)
    expect(stream.sample_rate).to eq(44_100)
    expect(stream.status).to eq(:stopped)
  end

  it 'requires on_get_data on a SoundStream' do
    expect { SFML::SoundStream.new(1, 44_100, [:mono]) }.to raise_error(NotImplementedError)
  end
end

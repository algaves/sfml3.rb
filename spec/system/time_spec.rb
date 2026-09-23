# frozen_string_literal: true

require_relative '../spec_helper'

# SF::System::Time shadows Ruby's core Time under `include SF::System`, so every constant
# below is namespace-qualified rather than resolved lexically.
RSpec.describe SF::System::Time do
  it 'constructs from the unit factories' do
    expect(SF::System::Time.seconds(1.5).as_seconds).to be_within(0.001).of(1.5)
    expect(SF::System::Time.milliseconds(1500).as_milliseconds).to eq(1500)
    expect(SF::System::Time.microseconds(2500).as_microseconds).to eq(2500)
    expect(SF::System::Time.zero.as_microseconds).to eq(0)
  end

  it 'supports arithmetic and comparison' do
    a = SF::System::Time.seconds(2)
    b = SF::System::Time.seconds(0.5)

    expect((a + b).as_seconds).to be_within(0.001).of(2.5)
    expect((a - b).as_seconds).to be_within(0.001).of(1.5)
    expect((a * 2).as_seconds).to be_within(0.001).of(4.0)
    expect(a).to be > b
    expect(a).to eq(SF::System::Time.seconds(2))
  end
end

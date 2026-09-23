# frozen_string_literal: true

require_relative '../spec_helper'

# SF::System::Clock measures elapsed wall time as an SF::System::Time, which shadows Ruby's
# core Time under `include SF::System`, so every constant below is namespace-qualified.
RSpec.describe SF::System::Clock do
  it 'reports a non-negative elapsed Time' do
    clock = SF::System::Clock.new
    elapsed = clock.elapsed_time
    expect(elapsed).to be_a(SF::System::Time)
    expect(elapsed.as_seconds).to be >= 0
  end

  it 'resets near zero on restart' do
    clock = SF::System::Clock.new
    t1 = clock.elapsed_time.as_seconds
    t2 = clock.restart!.as_seconds
    expect(t2).to be >= 0
    expect(t2).to be <= t1 + 0.01
  end

  it 'tracks its running state' do
    clock = SF::System::Clock.new
    expect(clock).to be_running
    clock.stop!
    expect(clock).not_to be_running
    clock.start!
    expect(clock).to be_running
  end

  it 'can be copied' do
    expect(SF::System::Clock.new.copy).to be_a(SF::System::Clock)
  end

  it 'times a block with Clock.measure' do
    elapsed = SF::System::Clock.measure { SF.sleep!(0.001) }

    expect(elapsed).to be_a(SF::System::Time)
    expect(elapsed.as_seconds).to be >= 0.0
  end
end

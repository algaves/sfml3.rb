# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::System::Sleep do
  it 'accepts a Time or a number' do
    expect(SF.sleep(0)).to be_within(0.001).of(0.0)
    expect(SF.sleep(SF::System::Time.zero)).to be_a(SF::System::Time)
  end

  it 'delegates the Sleep module to the top level' do
    expect(SF::System::Sleep.sleep!(0)).to be_within(0.001).of(0.0)
  end
end

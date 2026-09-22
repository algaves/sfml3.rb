# frozen_string_literal: true

require_relative '../spec_helper'

# The Rubyesque (Matz-like) window names and the window Style/State flag
# constants, plus Window's own deprecated aliases. Hardware-free: `allocate`
# gives window objects whose Ruby-only methods can run without a display.
RSpec.describe SF::Window::Window do
  describe 'Rubyesque names' do
    it 'exposes the primary Rubyesque names' do
      %i[open? focused? visible? request_focus! poll_events!].each do |name|
        expect(SF::Window::WindowBase.instance_methods).to include(name)
      end

      %i[clear! display! render!].each do |name|
        expect(SF::Window::Window.instance_methods).to include(name)
      end

      expect(SF::Window::Window).to respond_to(:open)
      expect(SF::Graphics::RenderWindow).to respond_to(:open)
      expect(SF::Graphics::RenderWindow.instance_methods).to include(:render!)
    end

    it 'gives the Rubyesque names precedence over the deprecated ones' do
      expect(SF::Window::WindowBase.instance_method(:open?).owner).to eq(SF::Window::WindowBase)
      expect(SF::Window::Window.instance_method(:open?).owner).to eq(SF::Window::Window)
      expect(SF::Graphics::RenderWindow.instance_method(:open?).owner).to eq(SF::Window::Window)
      expect(SF::Window::Window.instance_method(:clear!).owner).to eq(SF::Window::Window)
    end

    it 'keeps the deprecated window names around' do
      %i[is_open? focus? visible?].each do |name|
        expect(SF::Window::WindowBase.instance_methods).to include(name)
      end

      %i[is_open? focus? request_focus clear display].each do |name|
        expect(SF::Window::Window.instance_methods).to include(name)
      end
    end

    it 'defaults visible to true without a display' do
      expect(SF::Window::WindowBase.allocate).to be_visible
    end

    it 'returns an Enumerator from poll_events! without a block' do
      expect(SF::Window::WindowBase.allocate.poll_events!).to be_a(Enumerator)
    end

    it 'declares the block forms of the event methods' do
      expect(SF::Window::WindowBase.instance_method(:open!).owner).to eq(SF::Window::WindowBase)
      expect(SF::Window::Window.instance_method(:poll_event!).owner).to eq(SF::Window::Window)
      expect(SF::Window::Window.instance_method(:wait_event!).owner).to eq(SF::Window::Window)

      expect(SF::Window::WindowBase.instance_method(:poll_event!).parameters).to include(%i[opt event])
      expect(SF::Window::WindowBase.instance_method(:wait_event!).parameters).to include(%i[opt event])
    end
  end

  describe 'Style and State' do
    it 'are integer flag namespaces' do
      expect([SF::Window::Style::NONE, SF::Window::Style::TITLEBAR, SF::Window::Style::RESIZE,
              SF::Window::Style::CLOSE, SF::Window::Style::DEFAULT]).to eq([0, 1, 2, 4, 7])
      expect(SF::Window::Style::TITLEBAR | SF::Window::Style::RESIZE | SF::Window::Style::CLOSE)
        .to eq(SF::Window::Style::DEFAULT)

      expect(SF::Window::State::WINDOWED).to eq(0)
      expect(SF::Window::State::FULLSCREEN).to eq(1)
    end
  end
end

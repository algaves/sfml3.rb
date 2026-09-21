# frozen_string_literal: true

# A tiny immediate-mode GUI for the examples. Each frame the demo polls an
# Input, then calls the widget helpers with the current state and stores what
# they return -- there is no retained widget tree to route events through:
#
#   input = ExampleSupport::Gui::Input.new(window)
#
#   while window.is_open?
#     input.update
#     ExampleSupport::Gui.begin_frame(window)
#     ExampleSupport::Gui.panel(window, 'Controls', 40, 40, 300, 220)
#     show_grid = ExampleSupport::Gui.checkbox(window, input, 'Grid', 56, 90, 200, 26, show_grid)
#     volume = ExampleSupport::Gui.slider(window, input, 'Volume', 56, 150, 200, 18, volume, 0, 100)
#     window.close! if ExampleSupport::Gui.button(window, input, 'Quit', 56, 200, 100, 32)
#     window.display
#   end
#
# `examples/games/menu_demo.rb` is the worked example: a draggable control panel
# driving a live preview.

require 'sfml'
require_relative 'support'

module ExampleSupport
  module Gui
    FACE = [46, 52, 70, 255].freeze
    FACE_HOVER = [64, 72, 96, 255].freeze
    FACE_DOWN = [34, 40, 56, 255].freeze
    FRAME = [30, 34, 48, 245].freeze
    TITLE_BAR = [42, 48, 66, 255].freeze
    BORDER = [92, 102, 128, 255].freeze
    ACCENT = [96, 170, 240, 255].freeze
    TRACK = [24, 28, 40, 255].freeze
    TEXT = [232, 236, 246, 255].freeze
    MUTED = [156, 164, 186, 255].freeze
    TITLE_HEIGHT = 30

    # Edge-detected mouse state, updated once per frame from the real-time
    # Mouse, so `pressed?` / `released?` are true for exactly one frame.
    class Input
      attr_reader :x, :y

      def initialize(window)
        @window = window
        @x = 0.0
        @y = 0.0
        @down = false
        @was_down = false
      end

      def update
        position = SFML::Mouse.position(@window)
        @x = position.x
        @y = position.y
        @was_down = @down
        @down = SFML::Mouse.button_pressed?(:left)
      end

      def down?
        @down
      end

      def pressed?
        @down && !@was_down
      end

      def released?
        !@down && @was_down
      end

      def over?(x, y, width, height)
        @x.between?(x, x + width) && @y.between?(y, y + height)
      end
    end

    module_function

    # Resets the cursor to the system arrow; interactive widgets switch it to a
    # hand while hovered. Call once per frame before drawing.
    def begin_frame(window)
      window.mouse_cursor = arrow_cursor
    end

    def rectangle(window, x, y, width, height, fill, border = nil)
      box = SFML::RectangleShape.new([width, height])
      box.position = [x, y]
      box.fill_color = fill
      if border
        box.outline_thickness = 1
        box.outline_color = border
      end
      window.draw(box)
      box
    end

    def label(window, string, x, y, size: 15, color: TEXT)
      window.draw(ExampleSupport.text(string, size: size, position: [x, y], color: color))
    end

    def centred(window, string, size, color, x, y, width, height)
      text = ExampleSupport.text(string, size: size, color: color)
      bounds = text.local_bounds
      text.position = [x + ((width - bounds.width) / 2) - bounds.left,
                       y + ((height - bounds.height) / 2) - bounds.top]
      window.draw(text)
    end

    # A framed panel with a title bar. Returns the panel's top-left y offset so
    # callers can lay children out below the title.
    def panel(window, title, x, y, width, height)
      rectangle(window, x, y, width, height, FRAME, BORDER)
      rectangle(window, x, y, width, TITLE_HEIGHT, TITLE_BAR)
      label(window, title, x + 10, y + 7, size: 15)
      y + TITLE_HEIGHT
    end

    # A title bar that can be dragged. +state+ is a hash with :x, :y and
    # :dragging keys, so the panel position survives across frames.
    def draggable_panel(window, input, state, title, width, height)
      if input.pressed? && input.over?(state[:x], state[:y], width, TITLE_HEIGHT)
        state[:dragging] = true
        state[:grab_x] = input.x - state[:x]
        state[:grab_y] = input.y - state[:y]
      end
      state[:dragging] = false if input.released?

      if state[:dragging] && input.down?
        state[:x] = input.x - state[:grab_x]
        state[:y] = input.y - state[:grab_y]
      end

      panel(window, title, state[:x], state[:y], width, height)
    end

    # Returns true when clicked this frame.
    def button(window, input, string, x, y, width, height)
      hovered = input.over?(x, y, width, height)
      window.mouse_cursor = hand_cursor if hovered
      fill = hovered ? FACE_HOVER : FACE
      fill = FACE_DOWN if hovered && input.down?
      rectangle(window, x, y, width, height, fill, hovered ? ACCENT : BORDER)
      centred(window, string, 16, TEXT, x, y, width, height)
      hovered && input.pressed?
    end

    # Returns the (possibly toggled) checked state.
    def checkbox(window, input, string, x, y, width, height, checked)
      hovered = input.over?(x, y, width, height)
      window.mouse_cursor = hand_cursor if hovered
      box = height - 8
      rectangle(window, x, y + 4, box, box, hovered ? FACE_HOVER : FACE, hovered ? ACCENT : BORDER)
      rectangle(window, x + 4, y + 8, box - 8, box - 8, ACCENT) if checked
      text = ExampleSupport.text(string, size: 15, color: TEXT)
      bounds = text.local_bounds
      text.position = [x + height, y + ((height - bounds.height) / 2) - bounds.top]
      window.draw(text)
      return !checked if hovered && input.pressed?

      checked
    end

    # Returns the (possibly changed) value. Dragging stays live while the
    # pointer is held near the track, so it does not stop at the exact bounds.
    def slider(window, input, string, x, y, width, height, value, min, max, step: 1)
      hovered = input.over?(x - 8, y - 8, width + 16, height + 16)
      window.mouse_cursor = hand_cursor if hovered
      label(window, "#{string}  #{format_value(value, step)}", x, y - 20, size: 14, color: MUTED)
      rectangle(window, x, y + ((height - 8) / 2.0), width, 8, TRACK, BORDER)

      active = hovered && input.down?
      if active
        fraction = ((input.x - x) / width.to_f).clamp(0.0, 1.0)
        raw = min + (fraction * (max - min))
        value = step == 1 ? raw.round : (raw / step).round * step
      end

      fraction = (value - min) / (max - min).to_f
      handle = SFML::CircleShape.new(height / 2.0)
      handle.origin = [height / 2.0, height / 2.0]
      handle.position = [x + (fraction * width), y + (height / 2.0)]
      handle.fill_color = if active
                            ACCENT
                          else
                            (hovered ? FACE_HOVER : [150, 160, 190, 255])
                          end
      window.draw(handle)
      value
    end

    # A vertical list of single-choice options; returns the selected index.
    def radio(window, input, options, x, y, width, height, selected)
      row = height / options.length.to_f
      chosen = selected
      options.each_with_index do |option, index|
        top = y + (index * row)
        hovered = input.over?(x, top, width, row)
        window.mouse_cursor = hand_cursor if hovered
        marker = SFML::CircleShape.new((row / 2.0) - 3)
        marker.origin = [(row / 2.0) - 3, (row / 2.0) - 3]
        marker.position = [x + (row / 2.0), top + (row / 2.0)]
        marker.fill_color = if index == selected
                              ACCENT
                            else
                              (hovered ? FACE_HOVER : FACE)
                            end
        window.draw(marker)
        label(window, option, x + row, top + ((row - 18) / 2.0), size: 15,
                                                                 color: index == selected ? TEXT : MUTED)
        chosen = index if hovered && input.pressed?
      end
      chosen
    end

    def gauge(window, string, x, y, width, height, fraction)
      label(window, string, x, y - 20, size: 14, color: MUTED)
      rectangle(window, x, y, width, height, TRACK, BORDER)
      rectangle(window, x, y, width * fraction.clamp(0.0, 1.0), height, ACCENT)
    end

    def arrow_cursor
      @arrow_cursor ||= SFML::Cursor.from_system(:arrow)
    end

    def hand_cursor
      @hand_cursor ||= SFML::Cursor.from_system(:hand)
    end

    def format_value(value, step)
      step == 1 ? value.to_i.to_s : format('%.2f', value)
    end
  end
end

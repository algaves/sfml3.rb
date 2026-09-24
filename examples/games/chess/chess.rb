# frozen_string_literal: true

# Chess, hot-seat, with the full move rules: legal-move filtering (you cannot
# leave your king in check), check, checkmate, stalemate, castling, en passant
# and pawn promotion. Pieces are drawn as letters on discs, since the bundled
# font has no Unicode chess glyphs. Click a piece to see its legal targets,
# click a target to move; when a promotion has several choices a picker appears.
# U undoes, R restarts, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/chess/chess.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

def sound(name, volume: 60)
  buffer = SoundBuffer.from_file(File.join(ASSETS, "#{name}.wav"))
  Sound.new(buffer).tap { |sound| sound.volume = volume }
end

SQ = 68
BOARD_X = 24
BOARD_Y = 60
BOARD_PX = SQ * 8
WINDOW_W = BOARD_X + BOARD_PX + 244
WINDOW_H = BOARD_Y + BOARD_PX + 16
LIGHT = [228, 221, 205, 255].freeze
DARK = [132, 110, 86, 255].freeze
SELECTED = [246, 210, 90, 160].freeze
TARGET = [90, 180, 120, 150].freeze
CHECK = [220, 70, 70, 170].freeze
LAST_MOVE = [110, 160, 220, 90].freeze

GLYPHS = { king: 'K', queen: 'Q', rook: 'R', bishop: 'B', knight: 'N', pawn: 'P' }.freeze
BACK_RANK = %i[rook knight bishop queen king bishop knight rook].freeze
PAWN_FORWARD = { white: -1, black: 1 }.freeze
PAWN_START = { white: 6, black: 1 }.freeze
PAWN_PROMOTE = { white: 0, black: 7 }.freeze

def enemy_of(color)
  color == :white ? :black : :white
end

class ChessGame
  attr_reader :board, :turn, :state, :captured, :last_move

  def initialize
    reset
  end

  def reset
    @board = Array.new(8) { Array.new(8) }
    8.times do |x|
      @board[0][x] = { type: BACK_RANK[x], color: :black, moved: false }
      @board[1][x] = { type: :pawn, color: :black, moved: false }
      @board[6][x] = { type: :pawn, color: :white, moved: false }
      @board[7][x] = { type: BACK_RANK[x], color: :white, moved: false }
    end
    @turn = :white
    @en_passant = nil
    @captured = { white: [], black: [] }
    @history = []
    @state = :playing
    @last_move = nil
    update_state
  end

  def piece_at(x, y)
    @board[y][x]
  end

  # Every legal move for the piece on +from+, or [] when it is not the side to
  # move's piece.
  def moves_from(x, y)
    piece = @board[y][x]
    return [] unless piece && piece[:color] == @turn && @state == :playing

    legal_moves(@turn).select { |move| move[:from] == [x, y] }
  end

  def apply(move)
    @history << snapshot
    captured = perform!(@board, move)
    @captured[enemy_of(@turn)] << captured if captured
    @last_move = move
    @en_passant = double_push_square(move)
    @turn = enemy_of(@turn)
    update_state
  end

  def undo
    restore(@history.pop) unless @history.empty?
  end

  def in_check?(color = @turn)
    king_square = find_king(@board, color)
    king_square ? attacked?(@board, king_square, enemy_of(color)) : false
  end

  def insufficient_material?
    pieces = @board.flatten.compact
    return false if pieces.any? { |piece| %i[pawn rook queen].include?(piece[:type]) }

    minors = pieces.count { |piece| %i[bishop knight].include?(piece[:type]) }
    minors <= 1
  end

  def status_text
    case @state
    when :checkmate then "checkmate -- #{enemy_of(@turn)} wins"
    when :stalemate then 'stalemate -- draw'
    when :draw then 'draw (insufficient material)'
    else "#{@turn}'s move#{' (in check)' if in_check?}"
    end
  end

  private

  def snapshot
    { board: @board.map { |row| row.map { |piece| piece&.dup } }, turn: @turn,
      en_passant: @en_passant, captured: { white: @captured[:white].dup, black: @captured[:black].dup },
      last_move: @last_move, state: @state }
  end

  def restore(snap)
    @board = snap[:board]
    @turn = snap[:turn]
    @en_passant = snap[:en_passant]
    @captured = snap[:captured]
    @last_move = snap[:last_move]
    @state = snap[:state]
  end

  def find_king(board, color)
    board.each_with_index do |row, y|
      row.each_with_index do |piece, x|
        return [x, y] if piece && piece[:type] == :king && piece[:color] == color
      end
    end
    nil
  end

  def update_state
    @state = if legal_moves(@turn).empty?
               in_check? ? :checkmate : :stalemate
             elsif insufficient_material?
               :draw
             else
               :playing
             end
  end

  def legal_moves(color)
    pseudo_moves(color).select do |move|
      copy = @board.map { |row| row.map { |piece| piece&.dup } }
      perform!(copy, move)
      king = find_king(copy, color)
      king.nil? || !attacked?(copy, king, enemy_of(color))
    end
  end

  def pseudo_moves(color)
    moves = []
    @board.each_with_index do |row, y|
      row.each_with_index do |piece, x|
        next unless piece && piece[:color] == color

        moves.concat(piece_moves(@board, x, y, piece, color))
      end
    end
    moves
  end

  def piece_moves(board, x, y, piece, color)
    case piece[:type]
    when :pawn then pawn_moves(board, x, y, color)
    when :knight then step_moves(board, x, y, color,
                                 [[1, 2], [2, 1], [-1, 2], [-2, 1], [1, -2], [2, -1], [-1, -2], [-2, -1]])
    when :king then king_moves(board, x, y, piece, color)
    when :bishop then ray_moves(board, x, y, color, [[1, 1], [1, -1], [-1, 1], [-1, -1]])
    when :rook then ray_moves(board, x, y, color, [[1, 0], [-1, 0], [0, 1], [0, -1]])
    when :queen
      ray_moves(board, x, y, color, [[1, 1], [1, -1], [-1, 1], [-1, -1], [1, 0], [-1, 0], [0, 1], [0, -1]])
    else []
    end
  end

  def pawn_moves(board, x, y, color)
    moves = []
    forward = PAWN_FORWARD[color]
    one = y + forward
    if inside?(x, one) && board[one][x].nil?
      add_pawn_move(moves, x, y, x, one, color)
      two = y + (forward * 2)
      moves << { from: [x, y], to: [x, two] } if y == PAWN_START[color] && board[two][x].nil?
    end
    [-1, 1].each do |dx|
      target_x = x + dx
      next unless inside?(target_x, one)

      target = board[one][target_x]
      if target && target[:color] != color
        add_pawn_move(moves, x, y, target_x, one, color)
      elsif @en_passant == [target_x, one]
        moves << { from: [x, y], to: [target_x, one], en_passant: true }
      end
    end
    moves
  end

  def add_pawn_move(moves, from_x, from_y, to_x, to_y, color)
    if to_y == PAWN_PROMOTE[color]
      %i[queen rook bishop knight].each do |promotion|
        moves << { from: [from_x, from_y], to: [to_x, to_y], promotion: promotion }
      end
    else
      moves << { from: [from_x, from_y], to: [to_x, to_y] }
    end
  end

  def step_moves(board, x, y, color, offsets)
    offsets.filter_map do |dx, dy|
      target_x = x + dx
      target_y = y + dy
      next unless inside?(target_x, target_y)

      target = board[target_y][target_x]
      { from: [x, y], to: [target_x, target_y] } if target.nil? || target[:color] != color
    end
  end

  def ray_moves(board, x, y, color, directions)
    moves = []
    directions.each do |dx, dy|
      step_x = x + dx
      step_y = y + dy
      while inside?(step_x, step_y)
        target = board[step_y][step_x]
        if target.nil?
          moves << { from: [x, y], to: [step_x, step_y] }
        else
          moves << { from: [x, y], to: [step_x, step_y] } if target[:color] != color
          break
        end
        step_x += dx
        step_y += dy
      end
    end
    moves
  end

  def king_moves(board, x, y, piece, color)
    moves = step_moves(board, x, y, color, [[1, 0], [-1, 0], [0, 1], [0, -1], [1, 1], [1, -1], [-1, 1], [-1, -1]])
    return moves if piece[:moved] || attacked?(board, [x, y], enemy_of(color))

    [[:king, 7, 1], [:queen, 0, -1]].each do |side, rook_x, direction|
      rook = board[y][rook_x]
      next unless rook && rook[:type] == :rook && rook[:color] == color && !rook[:moved]

      between = direction.positive? ? ((x + 1)...rook_x).to_a : ((rook_x + 1)...x).to_a
      next unless between.all? { |column| board[y][column].nil? }
      next if between.any? { |column| attacked?(board, [column, y], enemy_of(color)) }

      moves << { from: [x, y], to: [x + (2 * direction), y], castle: side }
    end
    moves
  end

  def perform!(board, move)
    from_x, from_y = move[:from]
    to_x, to_y = move[:to]
    piece = board[from_y][from_x]
    captured = nil

    if move[:en_passant]
      captured = board[from_y][to_x]
      board[from_y][to_x] = nil
    else
      captured = board[to_y][to_x]
    end

    board[to_y][to_x] = piece
    board[from_y][from_x] = nil
    piece[:moved] = true
    piece[:type] = move[:promotion] if move[:promotion]

    if move[:castle]
      rook_from = move[:castle] == :king ? 7 : 0
      rook_to = move[:castle] == :king ? to_x - 1 : to_x + 1
      board[to_y][rook_to] = board[to_y][rook_from]
      board[to_y][rook_from] = nil
      board[to_y][rook_to][:moved] = true
    end

    captured
  end

  def double_push_square(move)
    from_y = move[:from][1]
    to_x, to_y = move[:to]
    piece = @board[to_y][to_x]
    return nil unless piece[:type] == :pawn && (to_y - from_y).abs == 2

    [to_x, (from_y + to_y) / 2]
  end

  def attacked?(board, square, by_color)
    board.each_with_index do |row, y|
      row.each_with_index do |piece, x|
        next unless piece && piece[:color] == by_color

        attacks = if piece[:type] == :pawn
                    pawn_attacks?(x, y, square, by_color)
                  else
                    attacks_square?(board, x, y, piece, square)
                  end
        return true if attacks
      end
    end
    false
  end

  def pawn_attacks?(x, y, square, color)
    forward = PAWN_FORWARD[color]
    [[x - 1, y + forward], [x + 1, y + forward]].include?(square)
  end

  def attacks_square?(board, x, y, piece, square)
    case piece[:type]
    when :knight
      [[1, 2], [2, 1], [-1, 2], [-2, 1], [1, -2], [2, -1], [-1, -2], [-2, -1]].include?([square[0] - x, square[1] - y])
    when :king
      (square[0] - x).abs <= 1 && (square[1] - y).abs <= 1
    else
      directions = { bishop: [[1, 1], [1, -1], [-1, 1], [-1, -1]], rook: [[1, 0], [-1, 0], [0, 1], [0, -1]],
                     queen: [[1, 1], [1, -1], [-1, 1], [-1, -1], [1, 0], [-1, 0], [0, 1], [0, -1]] }[piece[:type]]
      directions.any? { |dx, dy| ray_hits?(board, x, y, dx, dy, square) }
    end
  end

  def ray_hits?(board, x, y, dx, dy, square)
    step_x = x + dx
    step_y = y + dy
    while inside?(step_x, step_y)
      return true if [step_x, step_y] == square
      return false unless board[step_y][step_x].nil?

      step_x += dx
      step_y += dy
    end
    false
  end

  def inside?(x, y)
    x.between?(0, 7) && y.between?(0, 7)
  end
end

window = Window.new(VideoMode.new(WINDOW_W, WINDOW_H, 32), 'SFML chess')
window.frame_rate = 60
move_sound = sound('beep', volume: 35)
capture_sound = sound('eat', volume: 40)
end_sound = sound('explode', volume: 45)
status = text('', size: 18, position: [BOARD_X, 20])
help = text('click a piece, then a target   U undo   R restart   escape quits',
            size: 13, position: [BOARD_X, WINDOW_H - 14])

game = nil
selected = nil
pending = nil
last_game_state = nil

start_game = lambda do
  game = ChessGame.new
  selected = nil
  pending = nil
end

play_apply = lambda do |move|
  before = game.captured.values.sum(&:length)
  game.apply(move)
  game.captured.values.sum(&:length) > before ? capture_sound.play! : move_sound.play!
  selected = nil
  pending = nil
end
start_game.call

square_at = lambda do |point|
  x = ((point.x - BOARD_X) / SQ).floor
  y = ((point.y - BOARD_Y) / SQ).floor
  x.between?(0, 7) && y.between?(0, 7) ? [x, y] : nil
end

centered = lambda do |text, size, color, center_x, center_y|
  label = text(text, size: size, color: color)
  bounds = label.local_bounds
  label.position = [center_x - bounds.left - (bounds.width / 2), center_y - bounds.top - (bounds.height / 2)]
  label
end

draw_piece = lambda do |window, piece, x, y|
  center_x = BOARD_X + (x * SQ) + (SQ / 2.0)
  center_y = BOARD_Y + (y * SQ) + (SQ / 2.0)
  disc = CircleShape.new(SQ * 0.42)
  disc.origin = [SQ * 0.42, SQ * 0.42]
  disc.position = [center_x, center_y]
  disc.fill_color = piece[:color] == :white ? [242, 242, 238, 255] : [42, 44, 58, 255]
  disc.outline_thickness = 2
  disc.outline_color = piece[:color] == :white ? [90, 90, 100, 255] : [210, 210, 220, 255]
  window.draw(disc)
  letter_color = piece[:color] == :white ? [40, 42, 56, 255] : [240, 240, 245, 255]
  window.draw(centered.call(GLYPHS[piece[:type]], 30, letter_color, center_x, center_y))
end

while window.open?
  click = nil
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :r then start_game.call
      when :u
        pending = nil
        selected = nil
        game.undo
      end
    when 'mouse-button-pressed'
      click = event.mouse_button[:position] if event.mouse_button[:button] == :left
    end
  end

  if click
    if pending
      index = ((click.x - (BOARD_X + BOARD_PX + 20)) / 40).floor
      choice = %i[queen rook bishop knight][index] if index.between?(0,
                                                                     3) && click.y.between?(BOARD_Y + 40, BOARD_Y + 76)
      play_apply.call(pending[:moves].find { |candidate| candidate[:promotion] == choice }) if choice
    else
      square = square_at.call(click)
      if square
        x, y = square
        target_moves = selected ? game.moves_from(selected[0], selected[1]).select { |move| move[:to] == square } : []
        if target_moves.empty?
          selected = game.piece_at(x, y)&.then { |piece| piece[:color] == game.turn ? square : nil }
        elsif target_moves.length > 1
          pending = { moves: target_moves }
        else
          play_apply.call(target_moves.first)
        end
      end
    end
  end

  end_sound.play! if game && game.state != last_game_state && %i[checkmate stalemate draw].include?(game.state)
  last_game_state = game&.state

  window.clear!([26, 28, 38, 255])

  last = game.last_move
  8.times do |y|
    8.times do |x|
      rect = RectangleShape.new([SQ, SQ])
      rect.position = [BOARD_X + (x * SQ), BOARD_Y + (y * SQ)]
      rect.fill_color = (x + y).even? ? LIGHT : DARK
      rect.fill_color = LAST_MOVE if last && (last[:from] == [x, y] || last[:to] == [x, y])
      rect.fill_color = SELECTED if selected == [x, y]
      window.draw(rect)
    end
  end

  if selected
    seen = {}
    game.moves_from(selected[0], selected[1]).each do |move|
      next if seen[move[:to]]

      seen[move[:to]] = true
      dot = CircleShape.new(SQ * 0.16)
      dot.origin = [SQ * 0.16, SQ * 0.16]
      dot.position = [BOARD_X + (move[:to][0] * SQ) + (SQ / 2.0), BOARD_Y + (move[:to][1] * SQ) + (SQ / 2.0)]
      dot.fill_color = game.piece_at(move[:to][0], move[:to][1]) ? [220, 90, 90, 200] : TARGET
      window.draw(dot)
    end
  end

  if game.in_check?
    king = nil
    8.times do |x|
      8.times do |y|
        king = [x, y] if game.piece_at(x, y)&.dig(:type) == :king && game.piece_at(x, y)[:color] == game.turn
      end
    end
    if king
      warning = RectangleShape.new([SQ, SQ])
      warning.position = [BOARD_X + (king[0] * SQ), BOARD_Y + (king[1] * SQ)]
      warning.fill_color = CHECK
      window.draw(warning)
    end
  end

  8.times do |y|
    8.times do |x|
      piece = game.piece_at(x, y)
      draw_piece.call(window, piece, x, y) if piece
    end
  end

  sidebar_x = BOARD_X + BOARD_PX + 20
  window.draw(text('captured by white', size: 14, position: [sidebar_x, WINDOW_H - 300]))
  window.draw(text(game.captured[:black].map { |piece| GLYPHS[piece[:type]] }.join(' '),
                   size: 20, position: [sidebar_x, WINDOW_H - 278]))
  window.draw(text('captured by black', size: 14, position: [sidebar_x, WINDOW_H - 236]))
  window.draw(text(game.captured[:white].map { |piece| GLYPHS[piece[:type]] }.join(' '),
                   size: 20, position: [sidebar_x, WINDOW_H - 214]))

  if pending
    window.draw(text('promote to:', size: 16, position: [sidebar_x, BOARD_Y + 8]))
    %w[Q R B N].each_with_index do |letter, index|
      box = RectangleShape.new([36, 36])
      box.position = [sidebar_x + (index * 40), BOARD_Y + 40]
      box.fill_color = [240, 240, 240, 255]
      window.draw(box)
      window.draw(centered.call(letter, 24, [30, 30, 40, 255], sidebar_x + (index * 40) + 18, BOARD_Y + 58))
    end
  end

  status.string = game.status_text
  window.draw(status)
  window.draw(help)

  window.display!
end

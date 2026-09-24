# frozen_string_literal: true

# Produces the "parts" used to break an example's code into subsections on the
# Examples pages. An example that also has a Book recipe reuses that recipe's
# headings, line ranges and explanations, so the two stay in step; the rest are
# split mechanically from their own top-level structure (imports, constants,
# classes, methods, the main loop).
#
# Each part is tagged with a kind -- :declaration, :routine or :logic -- so the
# generator can group the code into what it declares, the routines it defines,
# and the logic it runs, and each part carries a one-line explanation.
module SFMLDocs
  # Looks up (or derives) the code parts for a script under examples/.
  module ExampleWalkthrough
    ROOT = File.expand_path('../..', __dir__)
    BOOK = File.join(ROOT, 'docs', 'book')

    EXAMPLE_TAG = /\{%\s*example\s+\w+\s+(\S+)\s+(\d+)-(\d+)\s*%\}/
    PART_HEADING = /\A###\s+\d+\.\s+(.+?)\s*\z/

    MIN_LINES = 4
    MAX_PARTS = 8

    # @param script [String] path relative to the repo root, e.g.
    #   "examples/games/snake/snake.rb"
    # @return [Hash] { book: book_page_basename_or_nil, parts: [{title:, first:, last:, kind:, note:}] }
    def self.parts(script)
      lines = File.readlines(File.join(ROOT, script))
      covered = book_parts[script]
      raw = covered ? covered[:parts] : split(lines)
      parts = raw.map do |part|
        kind = part[:kind] || kind_for(lines, part)
        part.merge(kind: kind, note: part[:note] || note_for(lines, part, kind))
      end
      { book: covered && covered[:book], parts: parts }
    end

    # Maps each example script that a Book recipe walks through to that page and
    # its part headings, line ranges and prose. Built once and cached.
    def self.book_parts
      @book_parts ||= parse_book
    end

    def self.parse_book
      index = {}
      Dir.glob(File.join(BOOK, '*.md')).each do |file|
        record(index, File.basename(file), File.readlines(file))
      end
      index
    end

    def self.record(index, slug, lines)
      heading = nil
      prose = []
      lines.each do |line|
        if line.start_with?('## ')
          heading = nil
          prose = []
        elsif (match = line.match(PART_HEADING))
          heading = match[1].strip
          prose = []
        elsif heading && (match = line.match(EXAMPLE_TAG))
          (index[match[1]] ||= { book: slug, parts: [] })[:parts] <<
            { title: heading, first: match[2].to_i, last: match[3].to_i, note: clean_note(prose) }
          prose = []
        elsif heading
          prose << line
        end
      end
    end

    def self.clean_note(prose)
      text = prose.join(' ').gsub(/\s+/, ' ').strip
      text.empty? ? nil : text
    end

    # --- classification ------------------------------------------------------

    def self.kind_for(lines, part)
      body = slice(lines, part)
      return :routine if body.any? { |line| line.match?(/\A(class|module|def)\s/) }
      return :declaration if body.any? { |line| declaration_line?(line) }

      :logic
    end

    def self.declaration_line?(line)
      line.match?(/\A(require|include)\b/) || line.match?(/\A[A-Z][A-Z0-9_]*\s*=/) ||
        line.match?(/\A[A-Z]\w*\s*=\s*(Struct\.new|::|\w+\.new\b)/)
    end

    def self.note_for(lines, part, kind)
      comment = leading_comment(lines, part)
      return comment if comment && comment.sub(/\.\z/, '') != part[:title]

      case kind
      when :declaration then declaration_note(lines, part)
      when :routine then routine_note(lines, part)
      else logic_note(lines, part)
      end
    end

    def self.declaration_note(lines, part)
      body = slice(lines, part)
      clauses = []
      clauses << 'loads the `sfml` binding and its subsystem modules' if body.any? do |line|
        line.match?(/\A(require|include)\b/)
      end
      names = body.filter_map { |line| line[/\A([A-Z][A-Z0-9_]*)\s*=/, 1] }.uniq.first(8)
      clauses << "fixes the constants #{names.map { |name| "`#{name}`" }.join(', ')}" unless names.empty?
      return 'Declarations the rest of the example is built on.' if clauses.empty?

      "#{upcase_first(clauses.join(', and '))}."
    end

    def self.upcase_first(text)
      text[0].upcase + text[1..].to_s
    end

    def self.routine_note(lines, part)
      body = slice(lines, part)
      name = body.filter_map { |line| line[/\A(?:class|module)\s+(\w+)/, 1] }.first
      return "The `#{name}` class: it holds the state and the methods that act on it." if name

      method = body.filter_map { |line| line[/\Adef\s+(\w+)/, 1] }.first
      return "Defines the `#{method}` helper, called from the logic below." if method

      'A helper routine the example defines once and reuses.'
    end

    def self.logic_note(lines, part)
      body = slice(lines, part)
      if body.any? { |line| line.match?(/\A(while|until)\b/) || line.match?(/\Aloop\b/) }
        'The main loop: poll events, update the state, draw and present each frame.'
      else
        'Run-time setup and updates -- the example’s behaviour rather than its definitions.'
      end
    end

    def self.leading_comment(lines, part)
      line = lines[part[:first] - 1].to_s.strip
      return nil unless line.start_with?('#')

      text = line.sub(/\A#+\s*/, '').sub(/\A-+\s*/, '').sub(/\s*-+\z/, '').strip
      text.empty? ? nil : "#{capitalize(text)}."
    end

    def self.slice(lines, part)
      lines[(part[:first] - 1)..(part[:last] - 1)] || []
    end

    # --- mechanical splitter -------------------------------------------------

    def self.split(lines)
      start = first_code_line(lines)
      return [] if start.nil?

      pieces = reduce(merge_small(segments(lines, start)))
      pieces.filter_map { |range| part(lines, range) }
    end

    def self.merge_small(segments)
      segments = segments.dup
      loop do
        index = segments.index { |first, last| last - first + 1 < MIN_LINES }
        break if index.nil? || segments.size == 1

        neighbour = index.zero? ? 1 : index - 1
        segments = merge_at(segments, [index, neighbour].min, [index, neighbour].max)
      end
      segments
    end

    def self.reduce(segments)
      segments = segments.dup
      while segments.size > MAX_PARTS
        index = (0...(segments.size - 1)).min_by do |k|
          (segments[k][1] - segments[k][0]) + (segments[k + 1][1] - segments[k + 1][0])
        end
        segments = merge_at(segments, index, index + 1)
      end
      segments
    end

    def self.merge_at(segments, low, high)
      merged = [segments[low][0], segments[high][1]]
      kept = segments.each_with_index.reject { |_, index| index == low || index == high }.map(&:first)
      kept.insert(low, merged)
    end

    def self.first_code_line(lines)
      lines.each_index do |index|
        line = lines[index]
        next if line.strip.empty? || line.start_with?('#')

        return index
      end
      nil
    end

    # Top-level blocks: a class/module, a method, the main loop, or a `# ---`
    # section marker after a blank line.
    def self.segments(lines, start)
      bounds = [start]
      ((start + 1)...lines.size).each { |index| bounds << index if boundary?(lines, index, start) }
      bounds.each_with_index.map do |first, position|
        last = (bounds[position + 1] || lines.size) - 1
        [first, last]
      end
    end

    def self.boundary?(lines, index, start)
      line = lines[index]
      return false if line.strip.empty? || line.start_with?(' ', "\t")
      return true if major_block?(line)

      index != start && section_marker?(line) && lines[index - 1].strip.empty?
    end

    def self.major_block?(line)
      line.match?(/\A(class|module|def)\s/) || line.match?(/\A(while|until)\b/) || line.match?(/\Aloop\b/)
    end

    # A `# --- title ---` banner, as used to separate helper blocks.
    def self.section_marker?(line)
      line.match?(/\A#\s*-{2,}/)
    end

    def self.part(lines, (first, last))
      body = lines[first..last] || []
      { title: title_for(body), first: first + 1, last: last + 1 }
    end

    def self.title_for(body)
      line = body.find { |candidate| !candidate.strip.empty? }&.strip
      return 'Setup' if line.nil?

      comment_title(line) || code_title(line)
    end

    def self.comment_title(line)
      return nil unless line.start_with?('#')

      text = line.sub(/\A#+\s*/, '').sub(/\A-+\s*/, '').sub(/\s*-+\z/, '').strip
      text.empty? ? nil : capitalize(text)
    end

    def self.code_title(line)
      case line
      when /\Adef\s+(\w+)/ then "The `#{Regexp.last_match(1)}` helper"
      when /\A(class|module)\s+(\w+)/ then capitalize(Regexp.last_match(2))
      when /\A(while|until)\b/, /\Aloop\b/ then 'The main loop'
      else 'Setup'
      end
    end

    def self.capitalize(text)
      text = text[0].upcase + text[1..].to_s
      text.length > 60 ? "#{text[0, 57]}..." : text
    end
  end
end

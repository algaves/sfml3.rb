# frozen_string_literal: true

# Liquid tags that inline a real example file into the docs, so the runnable
# scripts under examples/ and the rendered docs stay in sync:
#
#   {% example ruby examples/games/snake/snake.rb %}       -- the whole file
#   {% example ruby examples/games/snake/snake.rb 12-24 %} -- lines 12..24
#
# The path is relative to the repository root. Both tags render the selected
# lines syntax-highlighted (through SFMLDocs::CodeRenderer, the same Rouge setup
# Kramdown uses for `docs/learn/`), so a Book walkthrough quotes the exact lines
# it is explaining without copying them and the excerpts cannot drift from the
# script.

require 'cgi'

# Namespace for this site's custom Liquid tags.
module SFMLDocs
  # Renders a range of an example file (or the whole thing) as a code block.
  class ExampleTag < Liquid::Tag
    ROOT = File.expand_path('../..', __dir__) # repository root
    EXAMPLES_DIR = File.expand_path('examples', ROOT)

    def initialize(tag_name, markup, tokens)
      super
      words = markup.split(/\s+/)
      @language = words[0]&.match?(/\A[a-z0-9-]+\z/) ? words.shift : 'ruby'
      @first, @last = extract_range(words)
      @path = words.join(' ').strip
    end

    def render(_context)
      file = File.expand_path(@path, ROOT)
      unless file.start_with?(EXAMPLES_DIR + File::SEPARATOR) && File.file?(file)
        return "<p class=\"label label-red\">example not found: #{CGI.escapeHTML(@path)}</p>"
      end

      CodeRenderer.render(selected_lines(file).join, @language)
    end

    private

    # A trailing "12-24" (or "12..24") selects that one-based, inclusive range.
    def extract_range(words)
      match = words.last&.match(/\A(\d+)(?:-|\.\.)(\d+)\z/)
      return [nil, nil] unless match

      words.pop
      [match[1].to_i, match[2].to_i]
    end

    def selected_lines(file)
      lines = File.readlines(file)
      return lines unless @first

      last = @last || @first
      lines[(@first - 1)..(last - 1)] || lines
    end
  end

  Liquid::Template.register_tag('example', ExampleTag)
end

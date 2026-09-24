# frozen_string_literal: true

require 'cgi'
require 'rouge'

# Renders Ruby (or any language) as the same highlighted markup Kramdown emits
# for `docs/learn/` fenced code blocks, so code inlined from `examples/` in the
# Book and Examples sections is syntax-highlighted identically. Both the
# `{% example %}` tag and the Examples generator call this.
module SFMLDocs
  # Wraps Rouge output in Kramdown's block structure.
  module CodeRenderer
    # @param code [String] the source to highlight
    # @param language [String] a Rouge lexer name (e.g. "ruby")
    # @return [String] a `<div class="language-... highlighter-rouge">` block
    def self.render(code, language = 'ruby')
      lexer = Rouge::Lexer.find_fancy(language, code) || Rouge::Lexers::PlainText
      inner = Rouge::Formatters::HTMLLegacy.new(css_class: 'highlight').format(lexer.lex(code))
      %(<div class="language-#{language} highlighter-rouge">#{inner}</div>)
    rescue StandardError
      escaped = CGI.escapeHTML(code)
      %(<div class="language-#{language} highlighter-rouge"><div class="highlight">) +
        %(<pre class="highlight"><code>#{escaped}</code></pre></div></div>)
    end
  end
end

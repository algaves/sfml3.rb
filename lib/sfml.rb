# frozen_string_literal: true

require 'sfml/version'

begin
  # Binary gems ship one extension per Ruby ABI under lib/sfml/<major.minor>/.
  # A gem built from source installs a single lib/sfml/sfml_ext.so instead.
  require "sfml/#{RUBY_VERSION[/\d+\.\d+/]}/sfml_ext"
rescue LoadError
  require 'sfml/sfml_ext'
end

# Pure-Ruby idiomatic sugar over the native API. Loaded after the extension so
# it can reopen the classes it defines.
require 'sfml/sugar'

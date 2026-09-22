# frozen_string_literal: true

source 'https://rubygems.org'

gemspec

# Needed wherever the extension is built, including inside rake-compiler-dock.
gem 'rake'
gem 'rake-compiler'

group :test do
  gem 'rspec', '~> 3.13'
end

# Host-only: neither building nor testing the extension needs these, so the
# cross-compile containers and the floor CI job leave the group out.
group :development do
  gem 'rake-compiler-dock'
  gem 'rbs'
  gem 'rubocop'
  gem 'steep'
  gem 'yard'
end

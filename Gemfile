source 'https://rubygems.org'

gemspec

# Needed wherever the extension is built, including inside rake-compiler-dock.
gem 'rake'
gem 'rake-compiler'

# Host-only: the cross-compile containers install this Gemfile without the
# development group, so keep anything they don't need out of the default one.
group :development do
  gem 'minitest'
  gem 'rake-compiler-dock'
  gem 'rubocop'
end

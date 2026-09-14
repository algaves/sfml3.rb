source 'https://rubygems.org'

gemspec

# Needed wherever the extension is built, including inside rake-compiler-dock.
gem 'rake'
gem 'rake-compiler'

group :test do
  # minitest 6 requires Ruby >= 3.2, which would exclude the 2.7.8 floor the
  # gemspec promises -- and that floor is only meaningful if CI can test it.
  gem 'minitest', '~> 5.0'
end

# Host-only: neither building nor testing the extension needs these, so the
# cross-compile containers and the old-Ruby CI jobs leave the group out.
group :development do
  gem 'rake-compiler-dock'
  gem 'rubocop'
end

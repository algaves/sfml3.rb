# frozen_string_literal: true

require_relative 'lib/sfml/version'

# This describes the *source* gem. `rake gem:native` builds binary gems from this
# same spec, and rake-compiler rewrites part of it on the way through: it sets
# `platform`, narrows `required_ruby_version` to the ABI range it actually built,
# and clears `extensions`. `Rakefile`'s `cross_compiling` hook additionally drops
# every `ext/**` file, since a binary gem never runs extconf. See
# rakelib/package.rake for the platform list and the ABI assertion.
Gem::Specification.new do |s|
  s.name = 'sfml3-rb'
  s.version = SFML::VERSION
  s.summary = 'Ruby bindings for SFML 3, with precompiled binaries'
  s.description = 'Ruby bindings for SFML 3 via CSFML. Precompiled binaries are published for ' \
                  'common platforms; elsewhere SFML and CSFML are downloaded and built from ' \
                  'source at install time, so neither needs to be installed system-wide.'
  s.homepage = 'https://github.com/algaves/sfml3.rb'

  s.authors = ['Algaves', 'Diego-Sealtiel Valderrama']
  s.email = 'SealtielFreak@yandex.com'
  s.license = '0BSD'

  s.required_ruby_version = '>= 3.1'

  # `required_rubygems_version` is deliberately left unset. Pinning it to
  # >= 3.3.22 here would lock Ruby 3.1.0 out of the source gem, which shipped
  # RubyGems 3.3.3 (3.1.7 ships 3.3.27). The binary gems do not need it either:
  # rake-compiler stamps that constraint itself on any linux platform carrying a
  # libc suffix, which is what keeps a RubyGems too old to tell musl from glibc
  # apart off those gems. Anything set here would simply be ANDed with it.

  # No `homepage_uri`: it would repeat `source_code_uri`, and rubygems.org shows
  # only the first of any duplicated link (RubyGems warns about it at build time).
  # The `homepage` field above already provides that link.
  s.metadata = {
    'source_code_uri' => 'https://github.com/algaves/sfml3.rb',
    'bug_tracker_uri' => 'https://github.com/algaves/sfml3.rb/issues',
    'changelog_uri' => 'https://github.com/algaves/sfml3.rb/blob/main/CHANGELOG.md',
    'documentation_uri' => 'https://rubydoc.info/gems/sfml3-rb'
  }

  # TODO.md ships because it is the module-by-module record of what is actually
  # bound, which is what someone needs to decide whether this gem covers them.
  docs = %w[README.md CHANGELOG.md LICENSE.md TODO.md]

  # Globbed rather than taken from `git ls-files`: packaging stays independent of
  # git and of the checkout's state, which matters because `rake gem` also runs
  # inside the rake-compiler-dock containers against a bind-mounted repository.
  s.files = Dir.glob('ext/**/*.{h,c,rb}') +
            Dir.glob('lib/**/*.rb') +
            docs

  s.extra_rdoc_files = docs
  s.rdoc_options = ['--main', 'README.md']

  # Already the default, but worth stating beside `extensions`: RubyGems prepends
  # the built extension's directory to these at install time.
  s.require_paths = ['lib']

  s.extensions = ['ext/extconf.rb']
end

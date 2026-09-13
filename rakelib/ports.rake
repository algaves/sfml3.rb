require_relative '../ext/ports'

# Thin wrapper: the real implementation lives in ext/ports.rb so that it ships
# inside the gem and `gem install` can build its own dependencies. Keep logic
# there, not here.

namespace :ports do
  desc 'Download and verify the pinned FreeType, SFML and CSFML source tarballs'
  task :fetch do
    Ports::RECIPES.each { |recipe| Ports.fetch(recipe) }
  end

  desc 'Build FreeType, SFML and CSFML into ports/<target>'
  task :build do
    Ports.build!
  end

  desc 'Remove built ports for the current target, keeping downloaded tarballs'
  task :clean do
    rm_rf [Ports.build_root, Ports.prefix]
  end
end

desc 'Build the vendored FreeType, SFML and CSFML dependencies'
task ports: 'ports:build'

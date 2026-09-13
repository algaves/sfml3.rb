require_relative '../ext/ports'

# Thin wrapper: the real implementation lives in ext/ports.rb so that it ships
# inside the gem and `gem install` can build its own dependencies. Keep logic
# there, not here.

namespace :ports do
  desc 'Download and verify the pinned SFML and CSFML source tarballs'
  task :fetch do
    Ports::RECIPES.each { |recipe| Ports.fetch(recipe) }
  end

  desc 'Build SFML and CSFML into ports/<host>'
  task :build do
    Ports.build!
  end

  desc 'Remove built ports, keeping downloaded tarballs'
  task :clean do
    rm_rf [Ports::BUILD, Ports::PREFIX]
  end
end

desc 'Build the vendored SFML and CSFML dependencies'
task ports: 'ports:build'

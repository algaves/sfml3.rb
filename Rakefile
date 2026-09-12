require 'rake'
require_relative 'lib/sfml/version'

task default: 'all'

task :compile do
  sh 'ruby ext/extconf.rb'
  sh 'make -C ext'
end

task :uninstall do
  system "gem uninstall sfml"
end

task :build do
  system "gem build sfml.gemspec"
end

task :install do
  system "gem install sfml-#{SFML::VERSION}.gem"
end

task :test => :compile do
  sh 'ruby -Ilib -Iext test/sfml_test.rb'
end

task :rubocop do
  sh 'bundle exec rubocop'
end

task :all do
  Rake::Task['uninstall'].invoke
  Rake::Task['build'].invoke
  Rake::Task['install'].invoke
end
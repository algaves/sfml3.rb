require 'rake'
require_relative 'lib/sfml/version'

task default: 'all'

task :compile do
  sh 'ruby extconf.rb', chdir: 'ext'
  sh 'make', chdir: 'ext'

  # mkmf only applies the 'sfml/' prefix on install, so mirror the installed
  # gem layout here. That way `require 'sfml/sfml_ext'` resolves with -Ilib
  # alone, in development and once installed.
  mkdir_p 'lib/sfml'
  cp 'ext/sfml_ext.so', 'lib/sfml/sfml_ext.so'
end

task :uninstall do
  system "gem uninstall sfml3-rb"
end

task :build do
  system "gem build sfml3-rb.gemspec"
end

task :install do
  system "gem install sfml3-rb-#{SFML::VERSION}.gem"
end

task :test => :compile do
  sh 'ruby -Ilib test/sfml_test.rb'
end

task :rubocop do
  sh 'bundle exec rubocop'
end

task :all do
  Rake::Task['uninstall'].invoke
  Rake::Task['build'].invoke
  Rake::Task['install'].invoke
end
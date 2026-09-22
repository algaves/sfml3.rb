# frozen_string_literal: true

require 'rake/clean'
require 'rake/extensiontask'
require 'rspec/core/rake_task'
require 'rubygems/package_task'

require_relative 'ext/ports'

GEMSPEC = Gem::Specification.load(File.expand_path('sfml3-rb.gemspec', __dir__))

# Builds out of tree into tmp/ and copies the result to lib/sfml/, so `require
# 'sfml/sfml_ext'` resolves with -Ilib alone in development and once installed.
# Cross builds are driven from rakelib/package.rake, which runs this same task
# inside a rake-compiler-dock container.
Rake::ExtensionTask.new('sfml_ext', GEMSPEC) do |ext|
  ext.ext_dir = 'ext'
  ext.lib_dir = 'lib/sfml'

  # Recursive because the sources live in per-subsystem directories; the default
  # `*.{c,cc,cpp}` would see none of them. Headers (and the shared `.inc`
  # fragment) are included too: this list is only used as the rebuild
  # prerequisites (extensiontask.rb:187), and without them editing a header
  # rebuilds nothing.
  ext.source_pattern = '**/*.{c,h,inc}'
  ext.cross_compile = true
  ext.cross_platform = Ports::TARGETS.keys

  # A binary gem never runs extconf, so the sources and the ports recipe are
  # dead weight in it -- and shipping them would imply a build that can't happen.
  ext.cross_compiling { |spec| spec.files.reject! { |f| f.start_with?('ext/') } }
end

Gem::PackageTask.new(GEMSPEC).define

RSpec::Core::RakeTask.new(:test) do |t|
  t.pattern = 'spec/**/*_spec.rb'
  t.rspec_opts = %w[-Ilib]
end
task test: :compile

desc 'Run RuboCop'
task :rubocop do
  sh 'bundle exec rubocop'
end

CLEAN.include('tmp', 'lib/sfml/**/sfml_ext.{so,bundle,dll}')
CLOBBER.include('pkg', 'ports/build')

task default: :test

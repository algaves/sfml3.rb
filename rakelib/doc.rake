# frozen_string_literal: true

# API docs are written as YARD-flavoured comments directly above the
# rb_define_* calls in ext/**/*.c (YARD ships the same C-source doc-comment
# parser RDoc uses). See .yardopts for the source list and output options
# (YARD::CLI::Yardoc reads it automatically, so it isn't passed here), and
# sig/**/*.rbs for the accompanying RBS type signatures.
begin
  require 'yard'

  YARD::Rake::YardocTask.new(:yard)

  desc 'Build API docs (alias for yard)'
  task doc: :yard
rescue LoadError
  # Host-only, like rubocop: neither building nor testing the extension
  # needs it, so the cross-compile containers and the floor CI job leave
  # the :development group out.
end

begin
  require 'rbs'

  desc 'Validate sig/**/*.rbs'
  task :rbs do
    sh 'bundle exec rbs -I sig validate'
  end
rescue LoadError
  # See above.
end

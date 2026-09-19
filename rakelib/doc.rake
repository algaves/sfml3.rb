# frozen_string_literal: true

# API docs are YARD-flavoured comments in ext/**/*.c: class/module docs on the
# Init_* function, method docs on the C function each rb_define_* names. YARD
# ships the same C-source doc-comment parser RDoc uses. See .yardopts for the
# source list and output options (YARD::CLI::Yardoc reads it automatically, so
# it isn't passed here), and sig/**/*.rbs for the accompanying RBS signatures.
begin
  require 'yard'

  YARD::Rake::YardocTask.new(:yard)

  desc 'Build API docs (alias for yard)'
  task doc: :yard

  desc 'Fail if any method has no description prose (yard stats uses blank?, which passes for tags-only docs)'
  task 'doc:undoc' do
    YARD.parse(Dir.glob(File.expand_path('../ext/**/*.c', __dir__)))
    missing = YARD::Registry.all(:method).reject { |m| m.is_attribute? || !m.docstring.empty? }
    unless missing.empty?
      abort "Undocumented methods (#{missing.size}):\n" +
            missing.map { |m| "  #{m.path}  (#{m.file}:#{m.line})" }.join("\n")
    end
  end
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

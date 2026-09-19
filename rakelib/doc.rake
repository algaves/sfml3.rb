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

  desc 'Build the themed API docs for the GitHub Pages site'
  task 'doc:site' do
    # Same sources and output dir as `rake yard` (both come from .yardopts),
    # plus the custom template and assets. Kept out of .yardopts so the gem
    # does not have to ship them and rubydoc.info stays on the stock theme.
    sh 'bundle exec yardoc ' \
       '--template-path yard/templates ' \
       '--asset yard/assets/sfml.css:css/sfml.css ' \
       '--asset yard/assets/favicon.svg:favicon.svg'
  end

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

begin
  require 'steep'

  desc 'Type-check lib/ against sig/**/*.rbs'
  task :steep do
    sh 'bundle exec steep check'
  end
rescue LoadError
  # See above.
end

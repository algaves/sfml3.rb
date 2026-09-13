begin
  require 'rake_compiler_dock'
rescue LoadError
  # Only the host launches containers. Inside one, this gem is absent by
  # design and the tasks below would never be run anyway.
  return
end

require_relative '../ext/ports'

# Cross-compilation of the binary gems. Each platform runs in its own
# rake-compiler-dock container, which supplies the cross toolchain and a set of
# cross-compiled rubies; we supply the target-side SFML dependencies via
# script/provision.sh and then build the ports and the extension as usual.
#
# The container needs a working Docker or Podman; rake-compiler-dock finds
# either one. Images are large and are pulled on first use.
module CrossBuild
  # ABIs baked into every binary gem. All of these have to exist as cross
  # rubies in the image -- this set is the intersection across our platforms,
  # since the x64-mingw-ucrt image has no 3.0 cross ruby.
  RUBY_CC_VERSIONS = %w[3.1.7 3.2.11 3.3.11 3.4.9 4.0.2].freeze

  module_function

  # Podman on an SELinux host (Fedora, RHEL) cannot read a bind mount that
  # carries no container label, and rake-compiler-dock builds its -v flag
  # itself with no :z suffix to add one. Turning the label off for the
  # container is the narrower fix: relabelling the checkout with chcon would
  # outlive the build.
  def container_options
    options = ['--rm', '-i']
    options << '-t' if $stdin.tty?
    options += ['--security-opt', 'label=disable'] if selinux_enforcing?
    options
  end

  def selinux_enforcing?
    File.read('/sys/fs/selinux/enforce').strip == '1'
  rescue SystemCallError
    false
  end

  # rake-compiler-dock prefers docker and falls back to podman, so podman is
  # only in play when docker is absent.
  def rootless_podman?
    return false if Process.uid.zero?

    !which('docker') && which('podman')
  end

  def which(command)
    ENV.fetch('PATH', '').split(File::PATH_SEPARATOR)
       .any? { |dir| File.executable?(File.join(dir, command)) }
  end

  # SFML_TARGET is set inline rather than passed through docker -e, which
  # rake-compiler-dock only does for a fixed list of variables. provision.sh
  # runs through bash so the build does not depend on the executable bit
  # surviving the checkout.
  def script(platform)
    <<~SH
      bash script/provision.sh #{platform} &&
      export BUNDLE_WITHOUT=development &&
      bundle install --jobs 4 &&
      SFML_TARGET=#{platform} \
        RUBY_CC_VERSION=#{RUBY_CC_VERSIONS.join(':')} \
        bundle exec rake native:#{platform} gem
    SH
  end

  def build(platform)
    # Start this platform from scratch, so the gem cannot disagree with the
    # sources. Two kinds of staleness make that a real risk:
    #
    #   tmp/  make compares .c against .o and never notices that extconf.rb
    #         changed the compiler flags, so objects survive a reconfigure.
    #   pkg/  Gem::PackageTask stages into a directory and treats it as one
    #         file prerequisite; a directory's mtime says nothing about its
    #         contents, so an old staging directory is silently repacked.
    #
    # Neither costs anything in CI, which always starts from a fresh checkout.
    # The vendored ports are untouched -- they live in ports/, keyed by target.
    FileUtils.rm_rf(Dir.glob("pkg/*-#{platform}") + ["tmp/#{platform}"])

    # rake-compiler-dock's `runas` wrapper drops to an unprivileged user so the
    # files it writes are owned by the invoking user -- right for Docker, wrong
    # for rootless Podman, which already maps the invoking user to container
    # root. Under Podman that user both loses write access to the bind mount
    # and lands outside the image's sudo group, so provisioning fails; staying
    # root avoids both and still writes host-owned files.
    RakeCompilerDock.sh(script(platform), platform: platform,
                                          options: container_options,
                                          runas: !rootless_podman?)
  end
end

namespace :gem do
  Ports::TARGETS.each_key do |platform|
    desc "Build the #{platform} binary gem in rake-compiler-dock"
    task(platform) { CrossBuild.build(platform) }
  end

  desc 'Build binary gems for every supported platform'
  task native: Ports::TARGETS.keys.map { |platform| "gem:#{platform}" }

  desc 'Build the source gem and every binary gem'
  task all: [:gem, 'gem:native']
end

desc 'List the platforms gem:native builds for'
task :platforms do
  Ports::TARGETS.each do |platform, spec|
    puts format('%<platform>-20s %<triple>-30s %<os>s',
                platform: platform, triple: spec[:triple], os: spec[:os])
  end
end

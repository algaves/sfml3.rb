# frozen_string_literal: true

# Committed git hooks live in .githooks/; git runs them once the checkout
# points core.hooksPath there. No loader gem, no node_modules -- these two
# tasks are the whole installation story. See .githooks/pre-commit for what
# the hook itself runs.
namespace :githooks do
  desc 'Point this checkout at the committed .githooks/ hooks'
  task :install do
    FileUtils.chmod '+x', '.githooks/pre-commit'
    sh 'git', 'config', 'core.hooksPath', '.githooks'
  end

  desc 'Revert this checkout to the default .git/hooks directory'
  task :uninstall do
    sh 'git', 'config', '--unset', 'core.hooksPath'
  end
end

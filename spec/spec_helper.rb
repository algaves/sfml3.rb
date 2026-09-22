# frozen_string_literal: true

require 'sfml'

# Shared setup for every spec. Kept as a plain file (not a `*_spec.rb`) so the
# `spec/**/*_spec.rb` pattern that drives the suite does not pick it up as an
# example group. Only pure formatter/order options live here; anything that
# opens a display or audio device stays in the examples, which skip when the
# hardware is absent.
RSpec.configure do |config|
  config.expect_with :rspec do |expectations|
    expectations.include_chain_clauses_in_custom_matcher_descriptions = true
  end

  config.mock_with :rspec do |mocks|
    mocks.verify_partial_doubles = true
  end

  config.shared_context_metadata_behavior = :apply_to_host_groups
  config.disable_monkey_patching!
  config.order = :random
  Kernel.srand config.seed
end

# Shared matchers for every suite, replacing the Minitest helpers so the
# examples read like RSpec.
#
# `be_vec_in_epsilon` mirrors Minitest's relative `assert_in_epsilon`, which
# can never accept a near-zero result against an exact 0. Matrices are compared
# with an absolute tolerance instead: that is most of any transform matrix.
#
# `expected_as_array` is the whole argument list (the value plus any tolerance),
# so the expected value is its first element. The RSpec `expected` helper is
# avoided: it returns just the value for one argument but collapses to the
# argument list once a tolerance is passed too.
RSpec::Matchers.define :be_vec_in_epsilon do
  match do |actual|
    values = expected_as_array
    exp = values[0]
    epsilon = values[1] || 0.01

    exp.zip(actual).all? do |e, a|
      (e - a).abs <= (e.abs <= epsilon ? epsilon : epsilon * e.abs)
    end
  end

  failure_message do |actual|
    values = expected_as_array
    exp = values[0]
    epsilon = values[1] || 0.01

    "expected #{actual.inspect} to be within #{epsilon} (relative) of #{exp.inspect}"
  end
end

RSpec::Matchers.define :be_matrix_in_delta do
  match do |actual|
    values = expected_as_array
    exp = values[0]
    delta = values[1] || 1e-5

    exp.length == actual.length &&
      exp.zip(actual).all? { |e, a| (e - a).abs <= delta }
  end

  failure_message do |actual|
    values = expected_as_array
    exp = values[0]
    delta = values[1] || 1e-5

    "expected #{actual.inspect} to be within #{delta} (absolute) of #{exp.inspect}"
  end
end

# frozen_string_literal: true

require 'sfml'
require 'minitest/autorun'

# Shared assertions for every suite. Kept in a module rather than a base class
# so each file can define its own Minitest::Test subclass, and named so the
# `test/**/*_test.rb` glob that `rake test` uses does not pick it up as a suite.
module SFMLTestHelpers
  def assert_vec_in_epsilon(expected, actual, epsilon = 0.01)
    expected.zip(actual).each do |e, a|
      assert_in_epsilon e, a, epsilon
    end
  end

  # assert_in_epsilon is relative, so it can never accept a near-zero result
  # against an exact 0 -- which is most of any transform matrix. Matrices are
  # compared with an absolute tolerance instead.
  def assert_matrix_in_delta(expected, actual, delta = 1e-5)
    assert_equal expected.length, actual.length
    expected.zip(actual).each_with_index do |(e, a), i|
      assert_in_delta e, a, delta, "element #{i}"
    end
  end
end

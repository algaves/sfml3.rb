# frozen_string_literal: true

# Type-checks the pure-Ruby side (lib/) against sig/**. The native extension
# cannot be introspected, so its API lives only in the RBS signatures; this
# target proves those signatures are loadable and that lib/ agrees with them.
# spec/ is deliberately out of scope: type-checking it would need the rspec
# signatures from the RBS collection.
target :lib do
  check 'lib'
  signature 'sig'
end

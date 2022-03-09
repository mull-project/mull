add_library(mull-cxx-mutators
  CXX/ArithmeticMutators.cpp
  CXX/BitwiseMutators.cpp
  CXX/RelationalMutators.cpp
  CXX/TrivialCXXMutator.cpp
  CXX/NumberMutators.cpp
  CXX/RemoveNegation.cpp
  CXX/CallMutators.cpp
  CXX/LogicalAndToOr.cpp
  CXX/LogicalOrToAnd.cpp
  MutatorKind.cpp
  MutatorsFactory.cpp
  NegateConditionMutator.cpp
  ScalarValueMutator.cpp
  )

target_link_libraries(mull-cxx-mutators irm)

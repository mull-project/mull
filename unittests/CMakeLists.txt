function(add_mutang_unittest test_dirname)
  add_custom_target(MutangTestSuite)
  add_unittest(MutangTestSuite ${test_dirname} ${ARGN})
  set_target_properties(MutangTestSuite PROPERTIES FOLDER "Tests")
endfunction()

add_mutang_unittest(MutangUnitTests
  ContextTest.cpp
  MutationEngineTests.cpp
  MutationOperatorsTests.cpp
  MutationPointTests.cpp
  SimpleTestFinderTest.cpp
  TestRunnersTests.cpp
)

target_link_libraries(MutangUnitTests
  mutang
  LLVMAsmParser
  LLVMCore
  LLVMSupport
)


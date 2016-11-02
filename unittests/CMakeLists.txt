function(add_mutang_unittest test_dirname)
  add_custom_target(MutangTestSuite)
  set_target_properties(MutangTestSuite PROPERTIES FOLDER "Tests")
  add_unittest(MutangTestSuite ${test_dirname} ${ARGN})
endfunction()

add_mutang_unittest(MutangUnitTests
  CompilerTests.cpp
  ConfigParserTests.cpp
  ContextTest.cpp
  DriverTests.cpp
  ForkProcessSandboxTest.cpp
  MutationEngineTests.cpp
  MutationPointTests.cpp
  TestRunnersTests.cpp

  MutationOperators/MutationOperatorsTests.cpp
  MutationOperators/NegateConditionMutationOperatorTest.cpp

  SimpleTest/SimpleTestFinderTest.cpp

  GoogleTest/GoogleTestFinderTest.cpp

  TestModuleFactory.cpp
  TestModuleFactory.h
)

target_link_libraries(MutangUnitTests
  mutang
  LLVMAsmParser
  LLVMCore
  LLVMSupport
)

add_custom_command(TARGET MutangUnitTests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_CURRENT_LIST_DIR}/fixtures $<TARGET_FILE_DIR:MutangUnitTests>/fixtures)



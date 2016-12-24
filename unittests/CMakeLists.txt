function(add_mull_unittest test_dirname)
  add_custom_target(MullTestSuite)
  set_target_properties(MullTestSuite PROPERTIES FOLDER "Tests")
  add_unittest(MullTestSuite ${test_dirname} ${ARGN})
endfunction()

add_mull_unittest(MullUnitTests
  CompilerTests.cpp
  ConfigParserTests.cpp
  ContextTest.cpp
  DriverTests.cpp
  ForkProcessSandboxTest.cpp
  MutationEngineTests.cpp
  MutationPointTests.cpp
  TestRunnersTests.cpp
  UniqueIdentifierTests.cpp

  MutationOperators/MutationOperatorsTests.cpp
  MutationOperators/NegateConditionMutationOperatorTest.cpp
  MutationOperators/RemoveVoidFunctionMutationOperatorTest.cpp

  SimpleTest/SimpleTestFinderTest.cpp

  GoogleTest/GoogleTestFinderTest.cpp

  SQLiteReporterTest.cpp

  TestModuleFactory.cpp
  TestModuleFactory.h
)

target_link_libraries(MullUnitTests
  mull
  sqlite3
  LLVMAsmParser
  LLVMCore
  LLVMExecutionEngine
  LLVMSupport

  # FIXME: Should not be arch specific
  LLVMX86AsmParser
  LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
)

add_custom_command(TARGET MullUnitTests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_CURRENT_LIST_DIR}/fixtures $<TARGET_FILE_DIR:MullUnitTests>/fixtures)


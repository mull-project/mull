set(mull_unittests_sources
  CompilerTests.cpp
  ConfigParserTests.cpp
  ContextTest.cpp
  DriverTests.cpp
  ForkProcessSandboxTest.cpp
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

  ConfigParserTestFixture.h
)

if(MULL_SUPPORT_RUST)

set(mull_unittests_sources
  ${mull_unittests_sources}

  Rust/RustTestFinderTest.cpp
  Rust/RustTestRunnerTest.cpp
  Rust/Driver_Rust_Test.cpp
)

endif()

# The "add_mull_unittest" function uses the pattern for LLVM unit test targets
# based on "add_unittest" LLVM helper. Example of its usage can be found in
# ${LLVM_SOURCE_DIR}/unittests/CMakeLists.txt.
# Since Mull's unit tests do not live inside LLVM tree's "unittests" folder,
# we have to follow the pattern and create a custom unit test suite which is
# "MullTestSuite" to be able to add actual "MullUnitTests" target to it.
function(add_mull_unittest test_dirname)
  add_custom_target(MullTestSuite)
  set_target_properties(MullTestSuite PROPERTIES FOLDER "Tests")
  add_unittest(MullTestSuite ${test_dirname} ${ARGN})
endfunction()

add_mull_unittest(MullUnitTests
  ${mull_unittests_sources}
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


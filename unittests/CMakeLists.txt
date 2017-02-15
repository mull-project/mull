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

add_executable(MullUnitTests ${mull_unittests_sources})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(MullUnitTests
  mull

  ${MULL_DEPENDENCY_GOOGLETEST}
  ${MULL_DEPENDENCY_GOOGLETESTMAIN}
  ${MULL_DEPENDENCY_LLVMAsmParser}

  # FIXME: Should not be arch specific
  ${MULL_DEPENDENCY_LLVMX86AsmParser}
)

add_custom_command(TARGET MullUnitTests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_CURRENT_LIST_DIR}/fixtures $<TARGET_FILE_DIR:MullUnitTests>/fixtures)

set(mull_unittests_sources
  CompilerTests.cpp
  ConfigParserTests.cpp
  ContextTest.cpp
  DriverTests.cpp
  ForkProcessSandboxTest.cpp
  MutationPointTests.cpp
  ModuleLoaderTest.cpp
  DynamicCallTreeTests.cpp

  TestRunnersTests.cpp
  UniqueIdentifierTests.cpp

  MutationOperators/MutationOperatorsTests.cpp
  MutationOperators/NegateConditionMutationOperatorTest.cpp
  MutationOperators/RemoveVoidFunctionMutationOperatorTest.cpp
  MutationOperators/ScalarValueMutationOperatorTest.cpp

  SimpleTest/SimpleTestFinderTest.cpp

  GoogleTest/GoogleTestFinderTest.cpp

  SQLiteReporterTest.cpp

  TestModuleFactory.cpp
  TestModuleFactory.h

  ConfigParserTestFixture.h
)

set(mull_unittests_rpath "") # to be filled later

if(MULL_SUPPORT_RUST)
  set(mull_unittests_sources
    ${mull_unittests_sources}

    Rust/RustTestFinderTest.cpp
    Rust/RustTestRunnerTest.cpp
    Rust/Driver_Rust_Test.cpp
  )

  set(mull_unittests_rpath
    ${mull_unittests_rpath} ${MULL_RUST_LIB_PATH}
  )
endif()

add_executable(MullUnitTests ${mull_unittests_sources})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${MULL_SOURCE_DIR}/googletest/include")

target_link_libraries(MullUnitTests
  mull

  google-test

  ${MULL_DEPENDENCY_LLVM_LIBRARIES}

  ${CMAKE_THREAD_LIBS_INIT}
)

# compile flags
get_target_property(default_compile_flags MullUnitTests COMPILE_FLAGS)
if(NOT default_compile_flags)
  set(default_compile_flags "")
endif()
set(mull_unittests_compileflags ${default_compile_flags} ${LLVM_CXX_FLAGS})
set_target_properties(MullUnitTests
  PROPERTIES COMPILE_FLAGS
  "${mull_unittests_compileflags}"
)

# Link flags
get_target_property(default_link_flags MullUnitTests LINK_FLAGS)
if(NOT ${default_link_flags})
set(default_link_flags "")
endif()
set(mull_unittests_link_flags
  "${default_link_flags} ${LLVM_LINK_FLAGS}"
)
set_target_properties(MullUnitTests PROPERTIES LINK_FLAGS "${mull_unittests_link_flags}")

# @rpath
get_target_property(default_rpath MullUnitTests INSTALL_RPATH)
set(mull_unittests_rpath ${default_rpath} ${mull_unittests_rpath})
set_target_properties(MullUnitTests
  PROPERTIES INSTALL_RPATH
  "${mull_unittests_rpath}"
)

# This copies (symlinks) fixtures/*.bc files that used by unit tests.
add_custom_command(TARGET MullUnitTests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_CURRENT_LIST_DIR}/fixtures $<TARGET_FILE_DIR:MullUnitTests>/fixtures)

set(mull_unittests_sources
  CompilerTests.cpp
  ConfigParserTests.cpp
  ContextTest.cpp
  DriverTests.cpp
  ForkProcessSandboxTest.cpp
  MutationPointTests.cpp
  ModuleLoaderTest.cpp

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

  ${MULL_DEPENDENCY_LLVM}

  ${CMAKE_THREAD_LIBS_INIT}
)

# @rpath
get_target_property(default_rpath MullUnitTests INSTALL_RPATH)
set(mull_unittests_rpath ${default_rpath} ${mull_unittests_rpath})
set_target_properties(MullUnitTests
  PROPERTIES INSTALL_RPATH
  "${mull_unittests_rpath}"
)

# This is done so that CMake sets LLVM_LIB_PATH to the MullUnitTests executable
# because libLLVM cannot find libLTO on Mac OS.
# This probably is a bug in the LLVM 3.9 bottle.
get_target_property(mull_unittests_lflags MullUnitTests LINK_FLAGS)
if (NOT DEFINED ${mull_unittests_lflags})
  set(mull_unittests_lflags "")
endif()
set(mull_unittests_lflags
  ${mull_unittests_lflags} "-Wl,-rpath ${LLVM_LIB_PATH}")
set_target_properties(MullUnitTests
  PROPERTIES LINK_FLAGS
  "${mull_unittests_lflags}"
)

# This copies (symlinks) fixtures/*.bc files that used by unit tests.
add_custom_command(TARGET MullUnitTests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_CURRENT_LIST_DIR}/fixtures $<TARGET_FILE_DIR:MullUnitTests>/fixtures)

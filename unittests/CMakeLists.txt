generate_fixture_factory(FACTORY_HEADER)

set(mull_unittests_sources
  CompilerTests.cpp
  ConfigParserTests.cpp
  DriverTests.cpp
  ForkProcessSandboxTest.cpp
  MutationPointTests.cpp
  ModuleLoaderTest.cpp
  DynamicCallTreeTests.cpp
  MutatorsFactoryTests.cpp
  TesteesTests.cpp

  TestRunnersTests.cpp
  UniqueIdentifierTests.cpp
  TaskExecutorTests.cpp

  Mutators/MutatorsTests.cpp
  Mutators/NegateConditionMutatorTest.cpp
  Mutators/RemoveVoidFunctionMutatorTest.cpp
  Mutators/ScalarValueMutatorTest.cpp
  Mutators/ConditionalsBoundaryMutatorTests.cpp

  JunkDetection/CXXJunkDetectorTests.cpp

  SimpleTest/SimpleTestFinderTest.cpp

  GoogleTest/GoogleTestFinderTest.cpp

  CustomTestFramework/CustomTestRunnerTests.cpp
  CustomTestFramework/CustomTestFinderTests.cpp

  SQLiteReporterTest.cpp

  TestModuleFactory.cpp
  TestModuleFactory.h

  ConfigParserTestFixture.h
  ${FACTORY_HEADER}
)

get_filename_component(factory_include_dir ${FACTORY_HEADER} DIRECTORY)

add_executable(MullUnitTests EXCLUDE_FROM_ALL ${mull_unittests_sources})
target_link_libraries(MullUnitTests
  mull
  google-test
)
target_include_directories(MullUnitTests PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MULL_SOURCE_DIR}/googletest/include
  ${MULL_INCLUDE_DIRS}
  ${factory_include_dir}
)
set_target_properties(MullUnitTests PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
  LINK_FLAGS ${MULL_LINK_FLAGS}
)
get_property(dependencies GLOBAL PROPERTY TEST_FIXTURES_DEPENDENCIES)
add_dependencies(MullUnitTests ${dependencies})

# This copies (symlinks) fixtures/*.bc files that used by unit tests.
add_custom_command(TARGET MullUnitTests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_CURRENT_LIST_DIR}/fixtures $<TARGET_FILE_DIR:MullUnitTests>/fixtures)


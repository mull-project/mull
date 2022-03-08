add_subdirectory(fixtures)
generate_fixture_factory(FACTORY_HEADER)

include(GoogleTest)

set(mull_unittests_sources
  Helpers/InMemoryCompiler.cpp
  Helpers/InMemoryFixture.cpp
  Helpers/MutationTestBed.cpp
  Helpers/MutationArtefact.cpp
  Helpers/BitcodeLoader.cpp

  Mutations-E2E/CXX/Arithmetic/Mutation_Arithmetic_Add_Test.cpp
  Mutations-E2E/CXX/Arithmetic/Mutation_Arithmetic_UnaryMinusToNoop_Test.cpp
  Mutations-E2E/CXX/ConstAssignment/Mutation_ConstAssignment_AssignConst.cpp
  Mutations-E2E/CXX/ConstAssignment/Mutation_ConstAssignment_InitConst.cpp
  Mutations-E2E/CXX/RemoveNegationTest.cpp
  Mutations-E2E/Scalar/01_Mutation_Scalar_ReturnValue_Test.cpp
  Mutations-E2E/Scalar/02_Mutation_Scalar_BinaryOperand_Test.cpp

  MutationPointTests.cpp
  MutatorsFactoryTests.cpp

  TaskExecutorTests.cpp

  Mutators/NegateConditionMutatorTest.cpp
  Mutators/ScalarValueMutatorTest.cpp
  Mutators/ConditionalsBoundaryMutatorTests.cpp

  JunkDetection/CXXJunkDetectorTests.cpp

  MutationTestingElementsReporterTest.cpp

  TestModuleFactory.cpp
  TestModuleFactory.h

  ${FACTORY_HEADER}

  JunkDetection/CompilationDatabaseTests.cpp
  MutationFilters/MutationFilterTests.cpp
  MutationFilters/GitDiffReaderTests.cpp
)

get_filename_component(factory_include_dir ${FACTORY_HEADER} DIRECTORY)

add_executable(mull-tests EXCLUDE_FROM_ALL ${mull_unittests_sources})
gtest_discover_tests(mull-tests)
target_link_libraries(mull-tests
  mull
  google-test
  json11
  clangCodeGen
)

target_include_directories(mull-tests PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MULL_INCLUDE_DIRS}
  Helpers
  ${factory_include_dir}
  )
target_include_directories(mull-tests SYSTEM PRIVATE
  ${MULL_SOURCE_DIR}/vendor/googletest/include
  ${THIRD_PARTY_INCLUDE_DIRS}
)
set_target_properties(mull-tests PROPERTIES
  COMPILE_FLAGS ${MULL_CXX_FLAGS}
  )
get_property(dependencies GLOBAL PROPERTY TEST_FIXTURES_DEPENDENCIES)
add_dependencies(mull-tests ${dependencies})

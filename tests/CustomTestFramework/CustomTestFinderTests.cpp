#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTestFinder.h"
#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;
using namespace std;

TEST(CustomTestFinder, findTests) {
  Diagnostics diagnostics;
  Configuration configuration;
  configuration.bitcodePaths = { mull::fixtures::custom_test_distance_bc_path(),
                                 mull::fixtures::custom_test_main_bc_path(),
                                 mull::fixtures::custom_test_test_bc_path() };

  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(configuration, diagnostics);
  Program program({}, {}, std::move(bitcode));
  ASSERT_EQ(program.getStaticConstructors().size(), 1UL);
  ASSERT_EQ(program.getStaticConstructors().front()->getName(), "initGlobalVariable");

  vector<CustomTestDefinition> testDefinitions({
      CustomTestDefinition("failing", "failing_test", "mull", { "failing_test" }),
      CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" }),
  });

  CustomTestFinder testFinder(testDefinitions);

  vector<mull::Test> tests = testFinder.findTests(program);
  ASSERT_EQ(2U, tests.size());

  vector<mull::Test>::iterator searchResult;

  searchResult = find_if(tests.begin(), tests.end(), [&](mull::Test &test) {
    return test.getTestName() == "failing";
  });
  ASSERT_NE(searchResult, tests.end());

  mull::Test &failingTest = *searchResult;
  ASSERT_EQ(failingTest.getTestName(), "failing");
  ASSERT_EQ(failingTest.getProgramName(), "mull");
  ASSERT_EQ(failingTest.getArguments().size(), 1UL);
  ASSERT_EQ(failingTest.getArguments().front(), "failing_test");

  searchResult = find_if(tests.begin(), tests.end(), [&](mull::Test &test) {
    return test.getTestName() == "passing";
  });
  ASSERT_NE(searchResult, tests.end());

  mull::Test &passingTest = *searchResult;
  ASSERT_EQ(passingTest.getTestName(), "passing");
  ASSERT_EQ(passingTest.getProgramName(), "mull");
  ASSERT_EQ(passingTest.getArguments().size(), 1UL);
  ASSERT_EQ(passingTest.getArguments().front(), "passing_test");
}

TEST(CustomTestFinder, findTests_viaConfig) {
  Diagnostics diagnostics;
  Configuration configuration;
  configuration.bitcodePaths = { mull::fixtures::custom_test_distance_bc_path(),
                                 mull::fixtures::custom_test_main_bc_path(),
                                 mull::fixtures::custom_test_test_bc_path() };

  std::vector<CustomTestDefinition> testDefinitions({
      CustomTestDefinition("failing", "failing_test", "some_name", { "failing_test" }),
      CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" }),
  });

  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(configuration, diagnostics);
  Program program({}, {}, std::move(bitcode));
  ASSERT_EQ(program.getStaticConstructors().size(), 1UL);
  ASSERT_EQ(program.getStaticConstructors().front()->getName(), "initGlobalVariable");

  CustomTestFinder testFinder(testDefinitions);

  vector<mull::Test> tests = testFinder.findTests(program);
  ASSERT_EQ(2U, tests.size());

  vector<mull::Test>::iterator searchResult;

  searchResult = find_if(tests.begin(), tests.end(), [&](mull::Test &test) {
    return test.getTestName() == "failing";
  });
  ASSERT_NE(searchResult, tests.end());

  mull::Test &failingTest = *searchResult;
  ASSERT_EQ(failingTest.getTestName(), "failing");
  ASSERT_EQ(failingTest.getProgramName(), "some_name");
  ASSERT_EQ(failingTest.getArguments().size(), 1UL);
  ASSERT_EQ(failingTest.getArguments().front(), "failing_test");

  searchResult = find_if(tests.begin(), tests.end(), [&](mull::Test &test) {
    return test.getTestName() == "passing";
  });
  ASSERT_NE(searchResult, tests.end());

  mull::Test &passingTest = *searchResult;
  ASSERT_EQ(passingTest.getTestName(), "passing");
  ASSERT_EQ(passingTest.getProgramName(), "mull");
  ASSERT_EQ(passingTest.getArguments().size(), 1UL);
  ASSERT_EQ(passingTest.getArguments().front(), "passing_test");
}

TEST(CustomTestFinder, findTests_withEmptyConfig) {
  Diagnostics diagnostics;
  Configuration configuration;
  configuration.bitcodePaths = { mull::fixtures::custom_test_distance_bc_path(),
                                 mull::fixtures::custom_test_main_bc_path(),
                                 mull::fixtures::custom_test_test_bc_path() };

  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(configuration, diagnostics);
  Program program({}, {}, std::move(bitcode));

  std::vector<CustomTestDefinition> testDefinitions(
      { CustomTestDefinition("main", "main", "mull", {}) });
  CustomTestFinder testFinder(testDefinitions);

  vector<mull::Test> tests = testFinder.findTests(program);
  ASSERT_EQ(1U, tests.size());

  mull::Test &test = tests.front();
  ASSERT_EQ(test.getTestName(), "main");
  ASSERT_EQ(test.getProgramName(), "mull");
  ASSERT_EQ(test.getArguments().size(), 0UL);
}

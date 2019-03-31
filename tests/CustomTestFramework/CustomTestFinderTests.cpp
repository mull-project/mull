#include "FixturePaths.h"
#include "mull/Config/ConfigParser.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/ModuleLoader.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTestFinder.h"
#include "mull/TestFrameworks/CustomTestFramework/CustomTest_Test.h"

#include <llvm/IR/LLVMContext.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;
using namespace std;

TEST(CustomTestFinder, findTests) {
  Configuration configuration;
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);
  Program program({}, {}, std::move(loadedModules));

  vector<CustomTestDefinition> testDefinitions({
      CustomTestDefinition("failing", "failing_test", "mull", {"failing_test"}),
      CustomTestDefinition("passing", "passing_test", "mull", {"passing_test"}),
  });

  Filter filter;
  CustomTestFinder testFinder(testDefinitions);

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(program, filter);
  ASSERT_EQ(2U, tests.size());

  vector<unique_ptr<mull::Test>>::iterator searchResult;
  Function *constructor = nullptr;

  searchResult =
      find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
        return test.get()->getTestName() == "failing";
      });
  ASSERT_NE(searchResult, tests.end());

  CustomTest_Test *failingTest = dyn_cast<CustomTest_Test>(searchResult->get());
  ASSERT_NE(failingTest, nullptr);
  ASSERT_EQ(failingTest->getTestName(), "failing");
  ASSERT_EQ(failingTest->getProgramName(), "mull");
  ASSERT_EQ(failingTest->getArguments().size(), 1UL);
  ASSERT_EQ(*failingTest->getArguments().begin(), "failing_test");
  ASSERT_EQ(failingTest->getConstructors().size(), 1UL);
  constructor = *(failingTest->getConstructors().begin());
  ASSERT_EQ(constructor->getName(), "initGlobalVariable");

  searchResult =
      find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
        return test.get()->getTestName() == "passing";
      });
  ASSERT_NE(searchResult, tests.end());

  CustomTest_Test *passingTest = dyn_cast<CustomTest_Test>(searchResult->get());
  ASSERT_NE(passingTest, nullptr);
  ASSERT_EQ(passingTest->getTestName(), "passing");
  ASSERT_EQ(passingTest->getProgramName(), "mull");
  ASSERT_EQ(passingTest->getArguments().size(), 1UL);
  ASSERT_EQ(*passingTest->getArguments().begin(), "passing_test");
  ASSERT_EQ(passingTest->getConstructors().size(), 1UL);
  constructor = *(passingTest->getConstructors().begin());
  ASSERT_EQ(constructor->getName(), "initGlobalVariable");
}

TEST(CustomTestFinder, findTests_viaConfig) {
  const char *rawConfig = R"YAML(
custom_tests:
  - name:   failing
    method: failing_test
    program: some_name
    arguments: [ "failing_test" ]
  - name:   passing
    method: passing_test
    arguments: [ "passing_test" ]
  )YAML";

  yaml::Input input(rawConfig);

  ConfigParser parser;
  auto config = parser.loadConfig(input);
  Configuration configuration(config);
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);
  Program program({}, {}, std::move(loadedModules));

  Filter filter;
  CustomTestFinder testFinder(config.getCustomTests());

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(program, filter);
  ASSERT_EQ(2U, tests.size());

  vector<unique_ptr<mull::Test>>::iterator searchResult;
  Function *constructor = nullptr;

  searchResult =
      find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
        return test.get()->getTestName() == "failing";
      });
  ASSERT_NE(searchResult, tests.end());

  CustomTest_Test *failingTest = dyn_cast<CustomTest_Test>(searchResult->get());
  ASSERT_NE(failingTest, nullptr);
  ASSERT_EQ(failingTest->getTestName(), "failing");
  ASSERT_EQ(failingTest->getProgramName(), "some_name");
  ASSERT_EQ(failingTest->getArguments().size(), 1UL);
  ASSERT_EQ(*failingTest->getArguments().begin(), "failing_test");
  ASSERT_EQ(failingTest->getConstructors().size(), 1UL);
  constructor = *(failingTest->getConstructors().begin());
  ASSERT_EQ(constructor->getName(), "initGlobalVariable");

  searchResult =
      find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
        return test.get()->getTestName() == "passing";
      });
  ASSERT_NE(searchResult, tests.end());

  CustomTest_Test *passingTest = dyn_cast<CustomTest_Test>(searchResult->get());
  ASSERT_NE(passingTest, nullptr);
  ASSERT_EQ(passingTest->getTestName(), "passing");
  ASSERT_EQ(passingTest->getProgramName(), "mull");
  ASSERT_EQ(passingTest->getArguments().size(), 1UL);
  ASSERT_EQ(*passingTest->getArguments().begin(), "passing_test");
  ASSERT_EQ(passingTest->getConstructors().size(), 1UL);
  constructor = *(passingTest->getConstructors().begin());
  ASSERT_EQ(constructor->getName(), "initGlobalVariable");
}

TEST(CustomTestFinder, findTests_withFilter) {
  const char *rawConfig = R"YAML(
custom_tests:
  - name:   failing
    method: failing_test
    program: some_name
    arguments: [ "failing_test" ]
  - name:   passing
    method: passing_test
    arguments: [ "passing_test" ]
  )YAML";

  yaml::Input input(rawConfig);

  ConfigParser parser;
  auto config = parser.loadConfig(input);
  Configuration configuration(config);
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);
  Program program({}, {}, std::move(loadedModules));

  Filter filter;
  filter.includeTest("passing");
  CustomTestFinder testFinder(config.getCustomTests());

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(program, filter);
  ASSERT_EQ(1U, tests.size());

  vector<unique_ptr<mull::Test>>::iterator searchResult;
  Function *constructor = nullptr;

  searchResult =
      find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
        return test.get()->getTestName() == "passing";
      });
  ASSERT_NE(searchResult, tests.end());

  CustomTest_Test *passingTest = dyn_cast<CustomTest_Test>(searchResult->get());
  ASSERT_NE(passingTest, nullptr);
  ASSERT_EQ(passingTest->getTestName(), "passing");
  ASSERT_EQ(passingTest->getProgramName(), "mull");
  ASSERT_EQ(passingTest->getArguments().size(), 1UL);
  ASSERT_EQ(*passingTest->getArguments().begin(), "passing_test");
  ASSERT_EQ(passingTest->getConstructors().size(), 1UL);
  constructor = *(passingTest->getConstructors().begin());
  ASSERT_EQ(constructor->getName(), "initGlobalVariable");
}

TEST(CustomTestFinder, findTests_withEmptyConfig) {
  const char *rawConfig = "";

  yaml::Input input(rawConfig);

  ConfigParser parser;
  auto config = parser.loadConfig(input);
  Configuration configuration(config);
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};

  ModuleLoader loader;
  auto loadedModules = loader.loadModules(configuration);
  Program program({}, {}, std::move(loadedModules));

  Filter filter;
  CustomTestFinder testFinder(config.getCustomTests());

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(program, filter);
  ASSERT_EQ(1U, tests.size());

  CustomTest_Test *test = dyn_cast<CustomTest_Test>(tests.begin()->get());
  ASSERT_NE(test, nullptr);
  ASSERT_EQ(test->getTestName(), "main");
  ASSERT_EQ(test->getProgramName(), "mull");
  ASSERT_EQ(test->getArguments().size(), 0UL);
}

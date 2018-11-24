#include "CustomTestFramework/CustomTestFinder.h"

#include "Config/RawConfig.h"
#include "Config/ConfigParser.h"
#include "Context.h"
#include "CustomTestFramework/CustomTest_Test.h"
#include "TestModuleFactory.h"
#include "Filter.h"

#include <llvm/IR/LLVMContext.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;
using namespace std;

static TestModuleFactory SharedTestModuleFactory;
static LLVMContext context;

static vector<unique_ptr<MullModule>> loadTestModules() {
  function<vector<unique_ptr<MullModule>> ()> modules = [](){
    vector<unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };

  RawConfig config;
  config.normalizeParallelizationConfig();
  FakeModuleLoader loader(context, modules);
  return loader.loadModulesFromBitcodeFileList({""}, config);
}

TEST(CustomTestFinder, findTests) {
  auto loadedModules = loadTestModules();
  Context context;
  for (auto &module: loadedModules) {
    context.addModule(move(module));
  }

  vector<CustomTestDefinition> testDefinitions({
    CustomTestDefinition("failing", "failing_test", "mull", { "failing_test" }),
    CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" }),
  });

  Filter filter;
  CustomTestFinder testFinder(testDefinitions);

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(context, filter);
  ASSERT_EQ(2U, tests.size());

  vector<unique_ptr<mull::Test>>::iterator searchResult;
  Function *constructor = nullptr;

  searchResult = find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
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

  searchResult = find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
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
  auto loadedModules = loadTestModules();
  Context context;
  for (auto &module: loadedModules) {
    context.addModule(move(module));
  }

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

  Filter filter;
  CustomTestFinder testFinder(config.getCustomTests());

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(context, filter);
  ASSERT_EQ(2U, tests.size());

  vector<unique_ptr<mull::Test>>::iterator searchResult;
  Function *constructor = nullptr;

  searchResult = find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
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

  searchResult = find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
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
  auto loadedModules = loadTestModules();
  Context context;
  for (auto &module: loadedModules) {
    context.addModule(move(module));
  }

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

  Filter filter;
  filter.includeTest("passing");
  CustomTestFinder testFinder(config.getCustomTests());

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(context, filter);
  ASSERT_EQ(1U, tests.size());

  vector<unique_ptr<mull::Test>>::iterator searchResult;
  Function *constructor = nullptr;

  searchResult = find_if(tests.begin(), tests.end(), [&](unique_ptr<mull::Test> &test) {
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
  auto loadedModules = loadTestModules();
  Context context;
  for (auto &module: loadedModules) {
    context.addModule(move(module));
  }

  const char *rawConfig = "";

  yaml::Input input(rawConfig);

  ConfigParser parser;
  auto config = parser.loadConfig(input);

  Filter filter;
  CustomTestFinder testFinder(config.getCustomTests());

  vector<unique_ptr<mull::Test>> tests = testFinder.findTests(context, filter);
  ASSERT_EQ(1U, tests.size());

  CustomTest_Test *test = dyn_cast<CustomTest_Test>(tests.begin()->get());
  ASSERT_NE(test, nullptr);
  ASSERT_EQ(test->getTestName(), "main");
  ASSERT_EQ(test->getProgramName(), "mull");
  ASSERT_EQ(test->getArguments().size(), 0UL);
}

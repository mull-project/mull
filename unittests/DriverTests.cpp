#include "Config.h"
#include "Context.h"
#include "Driver.h"
#include "ModuleLoader.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "TestModuleFactory.h"
#include "TestResult.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static LLVMContext GlobalCtx;

static TestModuleFactory TestModuleFactory;

class FakeModuleLoader : public ModuleLoader {
public:
  FakeModuleLoader() : ModuleLoader(GlobalCtx) {}

  std::unique_ptr<MutangModule> loadModuleAtPath(const std::string &path) override {
    if (path == "foo") {
      auto module = TestModuleFactory.createTesterModule();
      return make_unique<MutangModule>(std::move(module), "");
    }

    else if (path == "bar") {
      auto module = TestModuleFactory.createTesteeModule();
      return make_unique<MutangModule>(std::move(module), "");
    }

    else if (path == "simple_test/negate_condition/tester") {
      auto module = TestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
      return make_unique<MutangModule>(std::move(module), "");
    }

    else if (path == "simple_test/negate_condition/testee") {
      auto module = TestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();
      return make_unique<MutangModule>(std::move(module), "");
    }

    return make_unique<MutangModule>(nullptr, "");
  }

};

TEST(Driver, SimpleTest_AddMutationOperator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  std::vector<std::string> ModulePaths({ "foo", "bar" });
  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  Config config(ModulePaths, doFork, dryRun, useCache, MutangDefaultTimeout, distance);

  FakeModuleLoader loader;

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  SimpleTestRunner runner;

  Driver Driver(config, loader, testFinder, runner);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto Results = Driver.Run();
  ASSERT_EQ(1u, Results.size());

  auto FirstResult = Results.begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, FirstResult->getOriginalTestResult().Status);
  ASSERT_EQ("test_count_letters", FirstResult->getTestName());

  auto &Mutants = FirstResult->getMutationResults();
  ASSERT_EQ(1u, Mutants.size());

  auto FirstMutant = Mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, FirstMutant->getExecutionResult().Status);

  ASSERT_NE(nullptr, FirstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_NegateConditionMutationOperator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  std::vector<std::string> ModulePaths({
    "simple_test/negate_condition/tester",
    "simple_test/negate_condition/testee"
  });

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  Config config(ModulePaths, doFork, dryRun, useCache, MutangDefaultTimeout, distance);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  FakeModuleLoader loader;
  SimpleTestRunner runner;

  Driver Driver(config, loader, testFinder, runner);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto Results = Driver.Run();
  ASSERT_EQ(1u, Results.size());

  auto FirstResult = Results.begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, FirstResult->getOriginalTestResult().Status);
  ASSERT_EQ("test_max", FirstResult->getTestName());

  auto &Mutants = FirstResult->getMutationResults();
  ASSERT_EQ(1u, Mutants.size());

  auto FirstMutant = Mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, FirstMutant->getExecutionResult().Status);

  ASSERT_NE(nullptr, FirstMutant->getMutationPoint());
}

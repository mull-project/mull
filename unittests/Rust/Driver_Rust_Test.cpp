#include "Config.h"
#include "Context.h"
#include "Driver.h"
#include "ModuleLoader.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "Result.h"
#include "Rust/RustTestFinder.h"
#include "Rust/RustTestRunner.h"
#include "TestModuleFactory.h"
#include "TestResult.h"

#include "Toolchain/Toolchain.h"

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

using namespace mull;
using namespace llvm;

static LLVMContext GlobalCtx;

static TestModuleFactory TestModuleFactory;

class FakeRustModuleLoader : public ModuleLoader {
public:
  FakeRustModuleLoader() : ModuleLoader(GlobalCtx) {}

  std::unique_ptr<MullModule> loadModuleAtPath(const std::string &path) override {
    if (path == "rust") {
      auto module = TestModuleFactory.rustModule();
      return make_unique<MullModule>(std::move(module), "rust");
    }

    return make_unique<MullModule>(nullptr, "");
  }
};

TEST(Driver_Rust, AddMutationOperator) {

  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  std::vector<std::string> ModulePaths({ "rust" });
  bool doFork = true;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";
  Config config(ModulePaths,
                {},
                {},
                doFork,
                dryRun,
                useCache,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  FakeRustModuleLoader loader;

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  RustTestFinder testFinder;

  Toolchain toolchain(config);
  llvm::TargetMachine &machine = toolchain.targetMachine();
  RustTestRunner runner(machine);

  Driver Driver(config, loader, testFinder, runner, toolchain);

  auto result = Driver.Run();
  ASSERT_EQ(4U, result->getTestResults().size());

  auto testResult_1 = result->getTestResults().begin()->get();
  auto testResult_2 = (result->getTestResults().begin() + 1)->get();
  auto testResult_3 = (result->getTestResults().begin() + 2)->get();
  auto testResult_4 = (result->getTestResults().begin() + 3)->get();

  ASSERT_EQ(ExecutionStatus::Passed,
            testResult_1->getOriginalTestResult().Status);
  ASSERT_EQ(ExecutionStatus::Passed,
            testResult_2->getOriginalTestResult().Status);
  ASSERT_EQ(ExecutionStatus::Passed,
            testResult_3->getOriginalTestResult().Status);
  ASSERT_EQ(ExecutionStatus::Passed,
            testResult_4->getOriginalTestResult().Status);

  ASSERT_EQ("_ZN7example17rusttest_foo_sum117h7aa5d6b570662192E",
            testResult_1->getTestName());
  ASSERT_EQ("_ZN7example17rusttest_foo_sum217h389da0550735b641E",
            testResult_2->getTestName());
  ASSERT_EQ("_ZN7example17rusttest_foo_sum317h792193082ea99887E",
            testResult_3->getTestName());
  ASSERT_EQ("_ZN7example17rusttest_foo_sum417h38f8b273364164edE",
            testResult_4->getTestName());

  auto &testResult_1_mutants = testResult_1->getMutationResults();
  auto &testResult_2_mutants = testResult_2->getMutationResults();
  auto &testResult_3_mutants = testResult_3->getMutationResults();
  auto &testResult_4_mutants = testResult_4->getMutationResults();

  ASSERT_EQ(1U, testResult_1_mutants.size());
  ASSERT_EQ(1U, testResult_2_mutants.size());
  ASSERT_EQ(1U, testResult_3_mutants.size());
  ASSERT_EQ(1U, testResult_4_mutants.size());

  auto testResult_1_mutant_1 = testResult_1_mutants.begin()->get();
  auto testResult_2_mutant_1 = testResult_2_mutants.begin()->get();
  auto testResult_3_mutant_1 = testResult_3_mutants.begin()->get();
  auto testResult_4_mutant_1 = testResult_4_mutants.begin()->get();

  ASSERT_EQ(ExecutionStatus::Crashed,
            testResult_1_mutant_1->getExecutionResult().Status);
  ASSERT_EQ(ExecutionStatus::Crashed,
            testResult_2_mutant_1->getExecutionResult().Status);
  ASSERT_EQ(ExecutionStatus::Crashed,
            testResult_3_mutant_1->getExecutionResult().Status);
  ASSERT_EQ(ExecutionStatus::Crashed,
            testResult_4_mutant_1->getExecutionResult().Status);

  ASSERT_NE(nullptr, testResult_1_mutant_1->getMutationPoint());
}

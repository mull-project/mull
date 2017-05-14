#include "Config.h"
#include "Context.h"
#include "Driver.h"
#include "ModuleLoader.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "Result.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "TestModuleFactory.h"
#include "TestResult.h"

#include "Toolchain/Toolchain.h"

#include <functional>
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

static TestModuleFactory SharedTestModuleFactory;

TEST(Driver, SimpleTest_AddMutationOperator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                doFork,
                dryRun,
                useCache,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    auto module1 = SharedTestModuleFactory.createTesterModule();
    modules.push_back(make_unique<MullModule>(std::move(module1), "1234"));

    auto module2 = SharedTestModuleFactory.createTesteeModule();
    modules.push_back(make_unique<MullModule>(std::move(module2), "5678"));

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  Toolchain toolchain(config);
  llvm::TargetMachine &machine = toolchain.targetMachine();
  SimpleTestRunner runner(machine);

  Driver Driver(config, loader, testFinder, runner, toolchain);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTestResults().size());

  auto FirstResult = result->getTestResults().begin()->get();
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

  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                doFork,
                dryRun,
                useCache,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    auto module1 = SharedTestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module();
    modules.push_back(make_unique<MullModule>(std::move(module1), "1234"));

    auto module2 = SharedTestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module();
    modules.push_back(make_unique<MullModule>(std::move(module2), "5678"));

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());

  Driver Driver(config, loader, testFinder, runner, toolchain);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTestResults().size());

  auto FirstResult = result->getTestResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, FirstResult->getOriginalTestResult().Status);
  ASSERT_EQ("test_max", FirstResult->getTestName());

  auto &Mutants = FirstResult->getMutationResults();
  ASSERT_EQ(1u, Mutants.size());

  auto FirstMutant = Mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, FirstMutant->getExecutionResult().Status);

  ASSERT_NE(nullptr, FirstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_RemoveVoidFunctionMutationOperator) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                doFork,
                dryRun,
                useCache,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    auto module1 = SharedTestModuleFactory.create_SimpleTest_RemoveVoidFunction_Tester_Module();
    modules.push_back(make_unique<MullModule>(std::move(module1), "1234"));

    auto module2 = SharedTestModuleFactory.create_SimpleTest_RemoveVoidFunction_Testee_Module();
    modules.push_back(make_unique<MullModule>(std::move(module2), "5678"));

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());

  Driver Driver(config, loader, testFinder, runner, toolchain);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTestResults().size());

  auto FirstResult = result->getTestResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, FirstResult->getOriginalTestResult().Status);
  ASSERT_EQ("test_func_with_a_void_function_inside", FirstResult->getTestName());

  auto &Mutants = FirstResult->getMutationResults();
  ASSERT_EQ(1u, Mutants.size());

  auto FirstMutant = Mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, FirstMutant->getExecutionResult().Status);

  ASSERT_NE(nullptr, FirstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_TesteePathCalculation) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                doFork,
                dryRun,
                useCache,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AddMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    auto module1 = SharedTestModuleFactory.create_SimpleTest_testeePathCalculation_tester();
    modules.push_back(make_unique<MullModule>(std::move(module1), "1234"));

    auto module2 = SharedTestModuleFactory.create_SimpleTest_testeePathCalculation_testee();
    modules.push_back(make_unique<MullModule>(std::move(module2), "5678"));

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());

  Driver Driver(config, loader, testFinder, runner, toolchain);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Passed state
  ///
  /// In this test we are only interested at reconstruction of testee path of a
  /// particular mutant.
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTestResults().size());

  auto firstResult = result->getTestResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getOriginalTestResult().Status);
  ASSERT_EQ("test_testee_path_calculation", firstResult->getTestName());

  auto &mutants = firstResult->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed, firstMutant->getExecutionResult().Status);
  ASSERT_NE(nullptr, firstMutant->getMutationPoint());

  auto &allTestees = result->getAllTestees();
  ASSERT_EQ(5U, allTestees.size());

  Testee *testee1 = allTestees[0].get();
  Testee *testee2 = allTestees[1].get();
  Testee *testee3 = allTestees[2].get();
  Testee *testee4 = allTestees[3].get();
  Testee *testee5 = allTestees[4].get();

  ASSERT_EQ(firstMutant->getTestee(), testee5);

  ASSERT_EQ(firstMutant->
            getTestee()->
            getCallerTestee(), testee4);

  ASSERT_EQ(firstMutant->
            getTestee()->
            getCallerTestee()->
            getCallerTestee(), testee3);

  ASSERT_EQ(firstMutant->
            getTestee()->
            getCallerTestee()->
            getCallerTestee()->
            getCallerTestee(), testee2);

  ASSERT_EQ(firstMutant->
            getTestee()->
            getCallerTestee()->
            getCallerTestee()->
            getCallerTestee()->
            getCallerTestee(), testee1);

  ASSERT_EQ(firstMutant->
            getTestee()->
            getCallerTestee()->
            getCallerTestee()->
            getCallerTestee()->
            getCallerTestee()->
            getCallerTestee(), nullptr);

  std::vector<std::string> callerPath = result.get()->calculateCallerPath(firstMutant);

  ASSERT_EQ(callerPath.size(), 5U);
  ASSERT_EQ(callerPath[0], "tester.c:6");
  ASSERT_EQ(callerPath[1], "testee.c:23");
  ASSERT_EQ(callerPath[2], "testee.c:19");
  ASSERT_EQ(callerPath[3], "testee.c:15");
  ASSERT_EQ(callerPath[4], "testee.c:8");
}

#include "Config.h"
#include "Context.h"
#include "Driver.h"
#include "ModuleLoader.h"
#include "MutationOperators/AddMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
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

#pragma mark - Mutation operators

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
  bool emitDebugInfo = false;
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
                emitDebugInfo,
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
  bool emitDebugInfo = false;
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
                emitDebugInfo,
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
  bool emitDebugInfo = false;
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
                emitDebugInfo,
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

TEST(Driver, SimpleTest_AndOrReplacementMutationOperator) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
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
                emitDebugInfo,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());

  SimpleTestFinder testFinder(std::move(mutationOperators));

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    auto module = SharedTestModuleFactory.create_SimpleTest_AndOrReplacement_Module();
    modules.push_back(make_unique<MullModule>(std::move(module), "1234"));

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());

  Driver Driver(config, loader, testFinder, runner, toolchain);

  auto result = Driver.Run();
  ASSERT_EQ(6U, result->getTestResults().size());

  /// Mutation #1: AND operator with 2 branches.
  {
    auto result1 = result->getTestResults().begin()->get();
    ASSERT_EQ(ExecutionStatus::Passed, result1->getOriginalTestResult().Status);
    ASSERT_EQ("test_AND_operator_2branches", result1->getTestName());

    auto &mutants1 = result1->getMutationResults();
    ASSERT_EQ(1U, mutants1.size());

    auto mutant1_1 = mutants1.begin()->get();
    ASSERT_EQ(ExecutionStatus::Failed, mutant1_1->getExecutionResult().Status);
  }

  /// Mutation #2: AND operator with 1 branch.
  {
    auto result2 = result->getTestResults()[1].get();
    ASSERT_EQ(ExecutionStatus::Passed, result2->getOriginalTestResult().Status);
    ASSERT_EQ("test_AND_operator_1branch", result2->getTestName());

    auto &mutants2 = result2->getMutationResults();
    ASSERT_EQ(1U, mutants2.size());

    auto mutant2_1 = mutants2.begin()->get();
    ASSERT_EQ(ExecutionStatus::Failed, mutant2_1->getExecutionResult().Status);
  }

  /// Mutation #3: OR operator with 2 branches.
  {
    auto result3 = result->getTestResults()[2].get();
    ASSERT_EQ(ExecutionStatus::Passed, result3->getOriginalTestResult().Status);
    ASSERT_EQ("test_OR_operator_2branches", result3->getTestName());

    auto &mutants3 = result3->getMutationResults();
    ASSERT_EQ(1U, mutants3.size());

    auto mutant3_1 = mutants3.begin()->get();
    ASSERT_EQ(ExecutionStatus::Failed, mutant3_1->getExecutionResult().Status);
  }

  /// Mutation #4: OR operator with 1 branch.
  {
    auto result4 = result->getTestResults()[3].get();
    ASSERT_EQ(ExecutionStatus::Passed, result4->getOriginalTestResult().Status);
    ASSERT_EQ("test_OR_operator_1branch", result4->getTestName());

    auto &mutants4 = result4->getMutationResults();
    ASSERT_EQ(1U, mutants4.size());

    auto mutant4_1 = mutants4.begin()->get();
    ASSERT_EQ(ExecutionStatus::Failed, mutant4_1->getExecutionResult().Status);
  }

  /// Mutation #5: Compound AND then OR expression.
  {
    auto result5 = result->getTestResults()[4].get();
    ASSERT_EQ(ExecutionStatus::Passed, result5->getOriginalTestResult().Status);
    ASSERT_EQ("test_compound_AND_then_OR_operator", result5->getTestName());

    auto &mutants5 = result5->getMutationResults();
    ASSERT_EQ(2U, mutants5.size());

    // Mutant 5.1 should pass because it is a relaxing AND -> OR replacement.
    auto mutant5_1 = mutants5[0].get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant5_1->getExecutionResult().Status);

    // Mutant 5.2 should pass because it is a stressing OR -> AND replacement.
    auto mutant5_2 = mutants5[1].get();
    ASSERT_EQ(ExecutionStatus::Failed, mutant5_2->getExecutionResult().Status);
  }

  /// Mutation #6: Compound OR then AND expression.
  {
    auto result6 = result->getTestResults()[5].get();
    ASSERT_EQ(ExecutionStatus::Passed, result6->getOriginalTestResult().Status);
    ASSERT_EQ("test_compound_OR_then_AND_operator", result6->getTestName());

    auto &mutants6 = result6->getMutationResults();
    ASSERT_EQ(2U, mutants6.size());

    /// Mutant 6.1 should pass because it is a stressing OR -> AND replacement
    /// in the first expression.
    auto mutant6_1 = mutants6[0].get();
    ASSERT_EQ(ExecutionStatus::Failed, mutant6_1->getExecutionResult().Status);

    /// Mutant 6.2 should pass because it is a relaxing AND -> OR replacement
    /// in the second expression.
    auto mutant6_2 = mutants6[1].get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_2->getExecutionResult().Status);
  }
}

#pragma mark - Testee path calculation

TEST(Driver, SimpleTest_TesteePathCalculation) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool doFork = false;
  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
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
                emitDebugInfo,
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

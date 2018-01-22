#include "Config.h"
#include "Context.h"
#include "Driver.h"
#include "Filter.h"
#include "ModuleLoader.h"
#include "MutationOperators/MathAddMutationOperator.h"
#include "MutationOperators/AndOrReplacementMutationOperator.h"
#include "MutationOperators/MathDivMutationOperator.h"
#include "MutationOperators/MathMulMutationOperator.h"
#include "MutationOperators/MathSubMutationOperator.h"
#include "MutationOperators/MutationOperatorsFactory.h"
#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/RemoveVoidFunctionMutationOperator.h"
#include "MutationOperators/ReplaceAssignmentMutationOperator.h"
#include "Result.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "TestModuleFactory.h"
#include "ExecutionResult.h"
#include "MutationsFinder.h"
#include "CustomTestFramework/CustomTestFinder.h"
#include "CustomTestFramework/CustomTestRunner.h"

#include "Toolchain/Toolchain.h"

#include <functional>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/Twine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory SharedTestModuleFactory;

#pragma mark - Mutation operators

TEST(Driver, SimpleTest_MathAddMutationOperator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;
  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_CountLettersTest_Module());
    modules.push_back(SharedTestModuleFactory.create_SimpleTest_CountLetters_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));

  SimpleTestFinder testFinder;

  Toolchain toolchain(config);
  llvm::TargetMachine &machine = toolchain.targetMachine();
  SimpleTestRunner runner(machine);
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_count_letters", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_MathSubMutationOperator) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver

  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_MathSub_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathSubMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  Toolchain toolchain(config);
  llvm::TargetMachine &machine = toolchain.targetMachine();
  SimpleTestRunner runner(machine);
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

    /// Given the modules we use here we expect:
    ///
    /// 1 original test, which has Passed state
    /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_math_sub", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_MathMulMutationOperator) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver

  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_MathMul_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathMulMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  Toolchain toolchain(config);
  llvm::TargetMachine &machine = toolchain.targetMachine();
  SimpleTestRunner runner(machine);
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

    /// Given the modules we use here we expect:
    ///
    /// 1 original test, which has Passed state
    /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_math_mul", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_MathDivMutationOperator) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver

  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_MathDiv_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathDivMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  Toolchain toolchain(config);
  llvm::TargetMachine &machine = toolchain.targetMachine();
  SimpleTestRunner runner(machine);
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

    /// Given the modules we use here we expect:
    ///
    /// 1 original test, which has Passed state
    /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_math_div", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
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

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<NegateConditionMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module());
    modules.push_back(SharedTestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_max", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_RemoveVoidFunctionMutationOperator) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<RemoveVoidFunctionMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_RemoveVoidFunction_Tester_Module());
    modules.push_back(SharedTestModuleFactory.create_SimpleTest_RemoveVoidFunction_Testee_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_func_with_a_void_function_inside", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_ANDORReplacementMutationOperator) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_ANDORReplacement_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

  auto result = Driver.Run();
  ASSERT_EQ(8U, result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: AND operator with 2 branches.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_AND_operator_2branches", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #2: AND operator with 1 branch.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_AND_operator_1branch", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #3: OR operator with 2 branches.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_OR_operator_2branches", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #4: OR operator with 1 branch.
  {
    auto mutant4 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant4->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_OR_operator_1branch", mutant4->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant4->getExecutionResult().status);
  }

  /// Mutation #5: Compound AND then OR expression.
  {
    // Mutant 5.1 should pass because it is a relaxing AND -> OR replacement.
    auto mutant5_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant5_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_OR_operator", mutant5_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant5_1->getExecutionResult().status);

    // Mutant 5.2 should pass because it is a stressing OR -> AND replacement.
    auto mutant5_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant5_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_OR_operator", mutant5_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant5_2->getExecutionResult().status);
  }

  /// Mutation #6: Compound AND then AND expression.
  {
    auto mutant6_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_AND_operator", mutant6_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant6_1->getExecutionResult().status);

    auto mutant6_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_AND_operator", mutant6_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_2->getExecutionResult().status);
  }

  /// Mutation #7: Compound OR then AND expression.
  {
    auto mutant7_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant7_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_AND_operator", mutant7_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant7_1->getExecutionResult().status);

    auto mutant7_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant7_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_AND_operator", mutant7_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant7_2->getExecutionResult().status);
  }

  /// Mutation #8: Compound OR then OR expression.
  {
    auto mutant8_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant8_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_OR_operator", mutant8_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant8_1->getExecutionResult().status);

    auto mutant8_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant8_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_OR_operator", mutant8_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant8_2->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, SimpleTest_ANDORReplacementMutationOperator_CPP) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool debugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                // this test crashes in 2 tests, so we want a sandbox.
                Config::Fork::Enabled,
                dryRun,
                useCache,
                debugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<AndOrReplacementMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_ANDORReplacement_CPPContent_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

  auto result = Driver.Run();
  ASSERT_EQ(6U, result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: OR operator
  {
    auto mutant1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z25test_OR_operator_with_CPPv", mutant1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant1->getExecutionResult().status);
  }

  /// Mutation #2: OR operator (PHI case)
  {
    auto mutant2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z34test_OR_operator_with_CPP_PHI_casev", mutant2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant2->getExecutionResult().status);
  }

  /// Mutation #3: OR operator (Assert)
  {
    auto mutant3 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant3->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z36test_OR_operator_with_CPP_and_assertv", mutant3->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Crashed, mutant3->getExecutionResult().status);
  }

  /// Mutation #4: AND operator
  {
    auto mutant4 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant4->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z26test_AND_operator_with_CPPv", mutant4->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant4->getExecutionResult().status);
  }

  /// Mutation #5: AND operator (PHI case)
  {
    auto mutant5 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant5->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z35test_AND_operator_with_CPP_PHI_casev", mutant5->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant5->getExecutionResult().status);
  }

  /// Mutation #6: AND operator (Assert)
  {
    auto mutant6 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant6->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z37test_AND_operator_with_CPP_and_assertv", mutant6->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Crashed, mutant6->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, SimpleTest_ReplaceAssignmentMutationOperator_CPP) {
  std::string projectName = "some_project";
  std::string testFramework = "SimpleTest";

  bool dryRun = false;
  bool useCache = false;
  bool debugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                {},
                Config::Fork::Disabled,
                dryRun,
                useCache,
                debugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<ReplaceAssignmentMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_ReplaceAssignment_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  SimpleTestRunner runner(toolchain.targetMachine());
  Filter filter;

  Driver Driver(config, loader, testFinder, runner, toolchain, filter, finder);

  auto result = Driver.Run();
  EXPECT_EQ(1U, result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  auto mutant1 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed, mutant1->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_replace_assignment", mutant1->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant1->getExecutionResult().status);

  auto mutant2 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed, mutant2->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_replace_assignment", mutant2->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant2->getExecutionResult().status);

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, customTest) {
  std::string projectName = "some_custom_project";
  std::string testFramework = "CustomTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  std::vector<CustomTestDefinition> testDefinitions({
    CustomTestDefinition("failing", "failing_test", "mull", { "failing_test" }),
    CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  });

  Config config("",
                projectName,
                testFramework,
                {},
                {},
                {},
                {},
                {},
                testDefinitions,
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  CustomTestFinder testFinder(config.getCustomTests());

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());
  Filter filter;
  filter.includeTest("passing");

  Driver driver(config, loader, testFinder, runner, toolchain, filter, finder);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(3UL, result->getMutationResults().size());

  auto mutants = result->getMutationResults().begin();

  auto mutant1 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed, mutant1->getTest()->getExecutionResult().status);
  ASSERT_EQ("passing", mutant1->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant1->getExecutionResult().status);
}

TEST(Driver, customTest_withDynamicLibraries) {
  std::string projectName = "some_custom_project_with_dylibs";
  std::string testFramework = "CustomTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  std::vector<CustomTestDefinition> testDefinitions({
    CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  });

  std::string dynamicLibrariesPath =
    TestModuleFactory::fixturePath("custom_test/dylibs_and_objects/dynamic_libraries.list");

  Config config("",
                projectName,
                testFramework,
                {},
                dynamicLibrariesPath,
                "",
                {},
                {},
                testDefinitions,
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  CustomTestFinder testFinder(config.getCustomTests());

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.createCustomTest_DylibsAndObjects_Test_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_DylibsAndObjects_Main_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());
  Filter filter;
  filter.includeTest("passing");

  Driver driver(config, loader, testFinder, runner, toolchain, filter, finder);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto test = result->getTests().begin()->get();
  ASSERT_NE(test, nullptr);
  ASSERT_EQ(ExecutionStatus::Passed, test->getExecutionResult().status);
  ASSERT_EQ("passing", test->getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, DISABLED_customTest_withDynamicLibraries_and_ObjectFiles) {
  std::string projectName = "some_custom_project_with_dylibs_and_object_files";
  std::string testFramework = "CustomTest";

  bool dryRun = false;
  bool useCache = false;
  bool emitDebugInfo = false;
  bool diagnostics = false;

  int distance = 10;
  std::string cacheDirectory = "/tmp/mull_cache";

  std::vector<CustomTestDefinition> testDefinitions({
    CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  });

  std::string dynamicLibrariesPath =
    TestModuleFactory::fixturePath("custom_test/dylibs_and_objects/dynamic_libraries.list");
  std::string objectFilesListPath =
    TestModuleFactory::fixturePath("custom_test/dylibs_and_objects/object_files.list");

  Config config("",
                projectName,
                testFramework,
                {},
                dynamicLibrariesPath,
                objectFilesListPath,
                {},
                {},
                testDefinitions,
                Config::Fork::Disabled,
                dryRun,
                useCache,
                emitDebugInfo,
                diagnostics,
                MullDefaultTimeoutMilliseconds,
                distance,
                cacheDirectory);

  std::vector<std::unique_ptr<MutationOperator>> mutationOperators;
  mutationOperators.emplace_back(make_unique<MathAddMutationOperator>());
  MutationsFinder finder(std::move(mutationOperators));
  CustomTestFinder testFinder(config.getCustomTests());

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.createCustomTest_DylibsAndObjects_Test_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(config);
  CustomTestRunner runner(toolchain.targetMachine());
  Filter filter;
  filter.includeTest("passing");

  Driver driver(config, loader, testFinder, runner, toolchain, filter, finder);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto test = result->getTests().begin()->get();
  ASSERT_NE(test, nullptr);
  ASSERT_EQ(ExecutionStatus::Passed, test->getExecutionResult().status);
  ASSERT_EQ("passing", test->getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

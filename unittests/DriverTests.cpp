#include "Config/Configuration.h"
#include "Context.h"
#include "Driver.h"
#include "Filter.h"
#include "ModuleLoader.h"
#include "Mutators/MathAddMutator.h"
#include "Mutators/AndOrReplacementMutator.h"
#include "Mutators/MathDivMutator.h"
#include "Mutators/MathMulMutator.h"
#include "Mutators/MathSubMutator.h"
#include "Mutators/MutatorsFactory.h"
#include "Mutators/NegateConditionMutator.h"
#include "Mutators/RemoveVoidFunctionMutator.h"
#include "Mutators/ReplaceAssignmentMutator.h"
#include "Result.h"
#include "SimpleTest/SimpleTestFinder.h"
#include "SimpleTest/SimpleTestRunner.h"
#include "TestModuleFactory.h"
#include "ExecutionResult.h"
#include "MutationsFinder.h"
#include "CustomTestFramework/CustomTestFinder.h"
#include "CustomTestFramework/CustomTestRunner.h"
#include "Toolchain/Mangler.h"

#include "JunkDetection/JunkDetector.h"
#include "Toolchain/Toolchain.h"
#include "Metrics/Metrics.h"

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

#pragma mark - Running Driver with no tests

TEST(Driver, RunningWithNoTests) {
  Configuration configuration;

  LLVMContext context;
  ModuleLoader loader;

  std::vector<std::unique_ptr<Mutator>> mutators;
  MutationsFinder finder(std::move(mutators), configuration);

  SimpleTestFinder testFinder;

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

  auto result = Driver.Run();
  ASSERT_EQ(0u, result->getTests().size());
  ASSERT_EQ(0u, result->getMutationResults().size());
}

#pragma mark - Mutators

TEST(Driver, SimpleTest_MathAddMutator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  Configuration configuration;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_CountLettersTest_Module());
    modules.push_back(SharedTestModuleFactory.create_SimpleTest_CountLetters_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  SimpleTestFinder testFinder;

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_MathSubMutator) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_MathSub_Module());

    return modules;
  };

  Configuration configuration;

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathSubMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_MathMulMutator) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_MathMul_Module());

    return modules;
  };

  Configuration configuration;

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathMulMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_MathDivMutator) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_MathDiv_Module());

    return modules;
  };

  Configuration configuration;

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathDivMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_NegateConditionMutator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then Run all the tests using driver

  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_NegateCondition_Tester_Module());
    modules.push_back(SharedTestModuleFactory.create_SimpleTest_NegateCondition_Testee_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_RemoveVoidFunctionMutator) {
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<RemoveVoidFunctionMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_RemoveVoidFunction_Tester_Module());
    modules.push_back(SharedTestModuleFactory.create_SimpleTest_RemoveVoidFunction_Testee_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_ANDORReplacementMutator) {
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<AndOrReplacementMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_ANDORReplacement_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_ANDORReplacementMutator_CPP) {
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<AndOrReplacementMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_ANDORReplacement_CPPContent_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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

TEST(Driver, SimpleTest_ReplaceAssignmentMutator_CPP) {
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ReplaceAssignmentMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  SimpleTestFinder testFinder;

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_SimpleTest_ReplaceAssignment_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  SimpleTestRunner runner(toolchain.mangler());
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver Driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

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
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("failing", "failing_test", "mull", {"failing_test"}),
      CustomTestDefinition("passing", "passing_test", "mull", {"passing_test"})
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  CustomTestFinder testFinder(configuration.customTests);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(3UL, result->getMutationResults().size());

  auto mutants = result->getMutationResults().begin();

  auto mutant = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
  ASSERT_EQ("passing", mutant->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
}

TEST(Driver, customTest_withDynamicLibraries) {
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  };
  configuration.dynamicLibraryPaths = {
      TestModuleFactory::fixturePath("custom_test/dylibs_and_objects/libdistance.dylib")
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  CustomTestFinder testFinder(configuration.customTests);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.createCustomTest_DylibsAndObjects_Test_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_DylibsAndObjects_Main_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto test = result->getTests().begin()->get();
  ASSERT_NE(test, nullptr);
  ASSERT_EQ(ExecutionStatus::Passed, test->getExecutionResult().status);
  ASSERT_EQ("passing", test->getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, junkDetector_enabled) {
  Configuration configuration;
  configuration.junkDetectionEnabled = true;
  configuration.customTests = {
      CustomTestDefinition("failing", "failing_test", "mull", { "failing_test" }),
      CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  CustomTestFinder testFinder(configuration.customTests);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  AllJunkDetector junkDetector;

  Driver driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, junkDetector_disabled) {
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("failing", "failing_test", "mull", { "failing_test" }),
      CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  CustomTestFinder testFinder(configuration.customTests);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.create_CustomTest_Distance_Distance_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Main_Module());
    modules.push_back(SharedTestModuleFactory.createCustomTest_Distance_Test_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  AllJunkDetector junkDetector;

  Driver driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(3UL, result->getMutationResults().size());
}

TEST(Driver, DISABLED_customTest_withDynamicLibraries_and_ObjectFiles) {
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("passing", "passing_test", "mull", { "passing_test" })
  };
  configuration.dynamicLibraryPaths = {
      TestModuleFactory::fixturePath("custom_test/dylibs_and_objects/libdistance.dylib")
  };
  configuration.objectFilePaths = {
      TestModuleFactory::fixturePath("custom_test/dylibs_and_objects/main.o")
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  CustomTestFinder testFinder(configuration.customTests);

  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = [](){
    std::vector<std::unique_ptr<MullModule>> modules;

    modules.push_back(SharedTestModuleFactory.createCustomTest_DylibsAndObjects_Test_Module());

    return modules;
  };

  LLVMContext context;
  FakeModuleLoader loader(context, modules);

  Toolchain toolchain(configuration);
  CustomTestRunner runner(toolchain.mangler());
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  Driver driver(configuration, loader, testFinder, runner, toolchain, filter, finder, metrics, junkDetector);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto test = result->getTests().begin()->get();
  ASSERT_NE(test, nullptr);
  ASSERT_EQ(ExecutionStatus::Passed, test->getExecutionResult().status);
  ASSERT_EQ("passing", test->getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

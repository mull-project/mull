#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/ExecutionResult.h"
#include "mull/Filter.h"
#include "mull/JunkDetection/JunkDetector.h"
#include "mull/Metrics/Metrics.h"
#include "mull/ModuleLoader.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/AndOrReplacementMutator.h"
#include "mull/Mutators/MathAddMutator.h"
#include "mull/Mutators/MathDivMutator.h"
#include "mull/Mutators/MathMulMutator.h"
#include "mull/Mutators/MathSubMutator.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/Mutators/ReplaceAssignmentMutator.h"
#include "mull/ObjectLoader.h"
#include "mull/Program/Program.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/TestFrameworkFactory.h"
#include "mull/Toolchain/Mangler.h"
#include "mull/Toolchain/Toolchain.h"

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

#pragma mark - Running Driver with no tests

TEST(Driver, RunningWithNoTests) {
  Configuration configuration;
  MutationsFinder finder({}, configuration);
  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  Program program({}, {}, {});
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

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
  configuration.bitcodePaths = {
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      fixtures::simple_test_count_letters_count_letters_bc_path()};
  configuration.forkEnabled = false;

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed,
            firstResult->getTest()->getExecutionResult().status);
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

  Configuration configuration;
  configuration.bitcodePaths = {fixtures::mutators_math_sub_module_bc_path()};

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathSubMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed,
            firstResult->getTest()->getExecutionResult().status);
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

  Configuration configuration;
  configuration.bitcodePaths = {fixtures::mutators_math_mul_module_bc_path()};

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathMulMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            firstResult->getTest()->getExecutionResult().status);
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

  Configuration configuration;
  configuration.bitcodePaths = {fixtures::mutators_math_div_module_bc_path()};

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathDivMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            firstResult->getTest()->getExecutionResult().status);
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
  configuration.bitcodePaths = {
      fixtures::mutators_negate_condition_testee_bc_path(),
      fixtures::mutators_negate_condition_tester_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<NegateConditionMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_max", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_RemoveVoidFunctionMutator) {
  Configuration configuration;
  configuration.bitcodePaths = {
      fixtures::mutators_remove_void_function_testee_bc_path(),
      fixtures::mutators_remove_void_function_tester_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<RemoveVoidFunctionMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = Driver.Run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_func_with_a_void_function_inside",
            firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, SimpleTest_ANDORReplacementMutator) {
  Configuration configuration;
  configuration.bitcodePaths = {
      fixtures::mutators_and_or_replacement_module_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<AndOrReplacementMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = Driver.Run();
  ASSERT_EQ(12U, result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: AND operator with 2 branches.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_AND_operator_2branches", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #2: AND operator with 1 branch.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_AND_operator_1branch", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #3: OR operator with 2 branches.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_OR_operator_2branches", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #4: OR operator with 1 branch.
  {
    auto mutant4 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant4->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_OR_operator_1branch", mutant4->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant4->getExecutionResult().status);
  }

  /// Mutation #5: Compound AND then OR expression.
  {
    // Mutant 5.1 should pass because it is a relaxing AND -> OR replacement.
    auto mutant5_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant5_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_OR_operator",
              mutant5_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant5_1->getExecutionResult().status);

    // Mutant 5.2 should pass because it is a stressing OR -> AND replacement.
    auto mutant5_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant5_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_OR_operator",
              mutant5_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant5_2->getExecutionResult().status);
  }

  /// Mutation #6: Compound AND then AND expression.
  {
    auto mutant6_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant6_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_AND_operator",
              mutant6_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant6_1->getExecutionResult().status);

    auto mutant6_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant6_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_AND_then_AND_operator",
              mutant6_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_2->getExecutionResult().status);
  }

  /// Mutation #7: Compound OR then AND expression.
  {
    auto mutant7_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant7_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_AND_operator",
              mutant7_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant7_1->getExecutionResult().status);

    auto mutant7_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant7_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_AND_operator",
              mutant7_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant7_2->getExecutionResult().status);
  }

  /// Mutation #8: Compound OR then OR expression.
  {
    auto mutant8_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant8_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_OR_operator",
              mutant8_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant8_1->getExecutionResult().status);

    auto mutant8_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant8_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("test_compound_OR_then_OR_operator",
              mutant8_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant8_2->getExecutionResult().status);
  }

  /// Edge case for Pattern #1: OR expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant1->getTest()->getExecutionResult().status);
    ASSERT_EQ(
        "test_OR_operator_always_scalars_case_with_function_call_pattern1",
        mutant1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getExecutionResult().status);

    auto mutant2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant2->getTest()->getExecutionResult().status);
    ASSERT_EQ(
        "test_OR_operator_always_scalars_case_with_function_call_pattern1",
        mutant2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getExecutionResult().status);
  }

  /// Edge case for Pattern #3: OR expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ(
        "test_OR_operator_always_scalars_case_with_function_call_pattern3",
        mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getExecutionResult().status);
  }

  /// Edge case for Pattern #1: AND expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant1->getTest()->getExecutionResult().status);
    ASSERT_EQ(
        "test_AND_operator_always_scalars_case_with_function_call_pattern1",
        mutant1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getExecutionResult().status);

    auto mutant2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant2->getTest()->getExecutionResult().status);
    ASSERT_EQ(
        "test_AND_operator_always_scalars_case_with_function_call_pattern1",
        mutant2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getExecutionResult().status);
  }

  /// Edge case for Pattern #3: AND expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ(
        "test_AND_operator_always_scalars_case_with_function_call_pattern3",
        mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, SimpleTest_ANDORReplacementMutator_CPP) {
  Configuration configuration;
  configuration.bitcodePaths = {
      fixtures::mutators_and_or_replacement_cpp_module_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<AndOrReplacementMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = Driver.Run();
  ASSERT_EQ(6U, result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: OR operator
  {
    auto mutant1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant1->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z25test_OR_operator_with_CPPv",
              mutant1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant1->getExecutionResult().status);
  }

  /// Mutation #2: OR operator (PHI case)
  {
    auto mutant2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant2->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z34test_OR_operator_with_CPP_PHI_casev",
              mutant2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant2->getExecutionResult().status);
  }

  /// Mutation #3: OR operator (Assert)
  {
    auto mutant3 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant3->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z36test_OR_operator_with_CPP_and_assertv",
              mutant3->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Crashed, mutant3->getExecutionResult().status);
  }

  /// Mutation #4: AND operator
  {
    auto mutant4 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant4->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z26test_AND_operator_with_CPPv",
              mutant4->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant4->getExecutionResult().status);
  }

  /// Mutation #5: AND operator (PHI case)
  {
    auto mutant5 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant5->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z35test_AND_operator_with_CPP_PHI_casev",
              mutant5->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant5->getExecutionResult().status);
  }

  /// Mutation #6: AND operator (Assert)
  {
    auto mutant6 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed,
              mutant6->getTest()->getExecutionResult().status);
    ASSERT_EQ("_Z37test_AND_operator_with_CPP_and_assertv",
              mutant6->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Crashed, mutant6->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, SimpleTest_ReplaceAssignmentMutator_CPP) {
  Configuration configuration;
  configuration.bitcodePaths = {
      fixtures::mutators_replace_assignment_module_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ReplaceAssignmentMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.simpleTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver Driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = Driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  auto mutant1 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            mutant1->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_replace_assignment", mutant1->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant1->getExecutionResult().status);

  auto mutant2 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            mutant2->getTest()->getExecutionResult().status);
  ASSERT_EQ("test_replace_assignment", mutant2->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant2->getExecutionResult().status);

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, customTest) {
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("failing", "failing_test", "mull", {"failing_test"}),
      CustomTestDefinition("passing", "passing_test", "mull",
                           {"passing_test"})};
  configuration.bitcodePaths = {mull::fixtures::custom_test_distance_bc_path(),
                                mull::fixtures::custom_test_main_bc_path(),
                                mull::fixtures::custom_test_test_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(3UL, result->getMutationResults().size());

  auto mutants = result->getMutationResults().begin();

  auto mutant = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed,
            mutant->getTest()->getExecutionResult().status);
  ASSERT_EQ("passing", mutant->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
}

TEST(Driver, customTest_withDynamicLibraries) {
  Configuration configuration;
  configuration.customTests = {CustomTestDefinition("passing", "passing_test",
                                                    "mull", {"passing_test"})};
  configuration.dynamicLibraryPaths = {
      fixtures::dylibs_and_objects_distance_dylib_path()};
  configuration.bitcodePaths = {fixtures::dylibs_and_objects_test_bc_path(),
                                fixtures::dylibs_and_objects_main_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program(configuration.dynamicLibraryPaths, {},
                  loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("passing", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, junkDetector_enabled) {
  Configuration configuration;
  configuration.junkDetectionEnabled = true;
  configuration.customTests = {
      CustomTestDefinition("failing", "failing_test", "mull", {"failing_test"}),
      CustomTestDefinition("passing", "passing_test", "mull",
                           {"passing_test"})};
  configuration.bitcodePaths = {fixtures::custom_test_distance_bc_path(),
                                fixtures::custom_test_main_bc_path(),
                                fixtures::custom_test_test_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  AllJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, junkDetector_disabled) {
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("failing", "failing_test", "mull", {"failing_test"}),
      CustomTestDefinition("passing", "passing_test", "mull",
                           {"passing_test"})};
  configuration.bitcodePaths = {fixtures::custom_test_distance_bc_path(),
                                fixtures::custom_test_main_bc_path(),
                                fixtures::custom_test_test_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader loader;
  Program program({}, {}, loader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  AllJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());
  ASSERT_EQ(3UL, result->getMutationResults().size());
}

TEST(Driver, customTest_withDynamicLibraries_and_ObjectFiles) {
  Configuration configuration;
  configuration.customTests = {CustomTestDefinition("passing", "passing_test",
                                                    "mull", {"passing_test"})};
  configuration.dynamicLibraryPaths = {
      fixtures::dylibs_and_objects_distance_dylib_path()};
  configuration.objectFilePaths = {fixtures::dylibs_and_objects_main_o_path()};
  configuration.bitcodePaths = {fixtures::dylibs_and_objects_test_bc_path()};

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  ModuleLoader moduleLoader;
  ObjectLoader objectLoader;
  Program program(configuration.dynamicLibraryPaths,
                  objectLoader.loadObjectFiles(configuration),
                  moduleLoader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("passing", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, DISABLED_customTest_withExceptions) {
  Configuration configuration;
  configuration.customTests = {
      CustomTestDefinition("passing", "_main", "mull", {}),
      CustomTestDefinition("passing", "main", "mull", {})};
  configuration.bitcodePaths = {fixtures::exceptions_main_bc_path()};

  MutationsFinder finder({}, configuration);

  ModuleLoader moduleLoader;
  ObjectLoader objectLoader;
  Program program(configuration.dynamicLibraryPaths,
                  objectLoader.loadObjectFiles(configuration),
                  moduleLoader.loadModules(configuration));

  Toolchain toolchain(configuration);
  Filter filter;
  filter.includeTest("passing");
  Metrics metrics;
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration));
  ForkTimerSandbox sandbox;

  Driver driver(configuration, sandbox, program, toolchain, filter, finder,
                metrics, junkDetector, testFramework);

  auto result = driver.Run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("passing", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

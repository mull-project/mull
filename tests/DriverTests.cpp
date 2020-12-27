#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/ExecutionResult.h"
#include "mull/Filters/JunkMutationFilter.h"
#include "mull/JunkDetection/JunkDetector.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/CXX/LogicalAndToOr.h"
#include "mull/Mutators/CXX/LogicalOrToAnd.h"
#include "mull/Mutators/RemoveVoidFunctionMutator.h"
#include "mull/ObjectLoader.h"
#include "mull/Program/Program.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/TestFrameworkFactory.h"
#include "mull/Toolchain/Toolchain.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>
#include <mull/Mutators/CXX/NumberMutators.h>
#include <mull/Mutators/CXX/RelationalMutators.h>

#include <functional>
#include <utility>

#include <gtest/gtest.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/Filters/FilePathFilter.h>
#include <mull/Filters/Filters.h>
#include <mull/Filters/NoDebugInfoFilter.h>

using namespace mull;
using namespace llvm;

#pragma mark - Running Driver with no tests

TEST(Driver, RunningWithNoTests) {
  Configuration configuration;
  Diagnostics diagnostics;
  MutationsFinder finder({}, configuration);
  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  Program program({}, {}, {});
  ForkTimerSandbox sandbox;
  Filters filters;
  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(0u, result->getTests().size());
  ASSERT_EQ(0u, result->getMutationResults().size());
}

#pragma mark - Mutators

TEST(Driver, MathAddMutator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then run all the tests using driver

  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::simple_test_count_letters_test_count_letters_bc_path(),
                                 fixtures::simple_test_count_letters_count_letters_bc_path() };

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::AddToSub>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  NullProcessSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = driver.run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, MathSubMutator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then run all the tests using driver

  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_math_sub_module_bc_path() };

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::SubToAdd>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = driver.run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();

  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, MathMulMutator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then run all the tests using driver
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_math_mul_module_bc_path() };

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::MulToDiv>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = driver.run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, MathDivMutator) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then run all the tests using driver
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_math_div_module_bc_path() };

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::DivToMul>());
  MutationsFinder finder(std::move(mutators), configuration);

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = driver.run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, CXXLessThanToGreaterOrEqual) {
  /// Create Config with fake BitcodePaths
  /// Create Fake Module Loader
  /// Initialize Driver using ModuleLoader and Config
  /// Driver should initialize (make them injectable? DI?)
  /// TestRunner and TestFinder based on the Config
  /// Then run all the tests using driver
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_negate_condition_testee_bc_path(),
                                 fixtures::mutators_negate_condition_tester_bc_path(),
                                 fixtures::mutators_negate_condition_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::LessThanToGreaterOrEqual>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = driver.run();
  ASSERT_EQ(1u, result->getTests().size());

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, RemoveVoidFunctionMutator) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_remove_void_function_testee_bc_path(),
                                 fixtures::mutators_remove_void_function_tester_bc_path(),
                                 fixtures::mutators_remove_void_function_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<RemoveVoidFunctionMutator>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  /// Given the modules we use here we expect:
  ///
  /// 1 original test, which has Passed state
  /// 1 mutant test, which has Failed state
  auto result = driver.run();
  ASSERT_EQ(1u, result->getTests().size());
  return;

  auto firstResult = result->getMutationResults().begin()->get();
  ASSERT_EQ(ExecutionStatus::Passed, firstResult->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", firstResult->getTest()->getTestName());

  auto &mutants = result->getMutationResults();
  ASSERT_EQ(1u, mutants.size());

  auto firstMutant = mutants.begin()->get();
  ASSERT_EQ(ExecutionStatus::Failed, firstMutant->getExecutionResult().status);

  ASSERT_NE(nullptr, firstMutant->getMutationPoint());
}

TEST(Driver, ANDORReplacementMutator) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_and_or_and_to_or_replacement_module_bc_path(),
                                 fixtures::mutators_and_or_and_to_or_replacement_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::LogicalAndToOr>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(1U, result->getTests().size());

  ASSERT_EQ(size_t(5), result->getMutationResults().size());

  auto mutants = result->getMutationResults().begin();

  /// Edge case for Pattern #1: AND expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getExecutionResult().status);

    auto mutant2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getExecutionResult().status);
  }

  /// Mutation #1: AND operator with 2 branches.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #2: AND operator with 1 branch.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Edge case for Pattern #3: AND expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, ORToANDReplacementMutator) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_and_or_or_to_and_replacement_module_bc_path(),
                                 fixtures::mutators_and_or_or_to_and_replacement_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::LogicalOrToAnd>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(size_t(1), result->getTests().size());
  ASSERT_EQ(size_t(5), result->getMutationResults().size());

  auto mutants = result->getMutationResults().begin();

  /// Edge case for Pattern #1: OR expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant1->getExecutionResult().status);

    auto mutant2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant2->getExecutionResult().status);
  }

  /// Mutation #1: OR operator with 2 branches.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #2: OR operator with 1 branch.
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Edge case for Pattern #3: OR expression that always evaluates to a scalar
  /// value but also contains a dummy function call (presence of a dummy
  /// function makes the Branch instruction to be generated).
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, ANDORReplacementMutator_CompoundOperators) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = {
    fixtures::mutators_and_or_and_or_replacement_compound_module_bc_path(),
    fixtures::mutators_and_or_and_or_replacement_compound_main_bc_path()
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::LogicalAndToOr>());
  mutators.emplace_back(std::make_unique<cxx::LogicalOrToAnd>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(size_t(1), result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: Compound AND then OR expression.
  {
    // Mutant 1.1 should pass because it is a relaxing AND -> OR replacement.
    auto mutant1_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant1_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant1_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant1_1->getExecutionResult().status);

    // Mutant 1.2 should pass because it is a stressing OR -> AND replacement.
    auto mutant1_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant1_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant1_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant1_2->getExecutionResult().status);
  }

  /// Mutation #2: Compound AND then AND expression.
  {
    auto mutant6_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant6_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant6_1->getExecutionResult().status);

    auto mutant6_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant6_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant6_2->getExecutionResult().status);
  }

  /// Mutation #3: Compound OR then AND expression.
  {
    auto mutant3_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant3_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant3_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant3_1->getExecutionResult().status);

    auto mutant3_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant3_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant3_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant3_2->getExecutionResult().status);
  }

  /// Mutation #4: Compound OR then OR expression.
  {
    auto mutant4_1 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant4_1->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant4_1->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant4_1->getExecutionResult().status);

    auto mutant4_2 = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant4_2->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant4_2->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Passed, mutant4_2->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, ANDToORReplacementMutator_CPP) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = {
    fixtures::mutators_and_or_and_to_or_replacement_cpp_module_bc_path(),
    fixtures::mutators_and_or_and_to_or_replacement_cpp_main_bc_path()
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::LogicalAndToOr>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(size_t(1), result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: AND operator
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #2: AND operator (PHI case)
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #3: AND operator (Assert)
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Crashed, mutant->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, ORToANDReplacementMutator_CPP) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = {
    fixtures::mutators_and_or_or_to_and_replacement_cpp_module_bc_path(),
    fixtures::mutators_and_or_or_to_and_replacement_cpp_main_bc_path()
  };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::LogicalOrToAnd>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;
  NoDebugInfoFilter debugInfoFilter;
  filters.mutationFilters.push_back(&debugInfoFilter);

  FilePathFilter filePathFilter;
  filePathFilter.exclude("include/c++/v1");
  filters.mutationFilters.push_back(&filePathFilter);

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(size_t(1), result->getTests().size());

  auto mutants = result->getMutationResults().begin();

  /// Mutation #1: OR operator
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #2: OR operator (PHI case)
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Failed, mutant->getExecutionResult().status);
  }

  /// Mutation #3: OR operator (Assert)
  {
    auto mutant = (mutants++)->get();
    ASSERT_EQ(ExecutionStatus::Passed, mutant->getTest()->getExecutionResult().status);
    ASSERT_EQ("main", mutant->getTest()->getTestName());
    ASSERT_EQ(ExecutionStatus::Crashed, mutant->getExecutionResult().status);
  }

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

class FunctionNameFilter : public FunctionFilter {
public:
  explicit FunctionNameFilter(std::string only) : only(std::move(only)) {}
  bool shouldSkip(llvm::Function *function) override {
    return function->getName() != only;
  }
  std::string name() override {
    return "test_filter";
  }

private:
  std::string only;
};

TEST(Driver, ReplaceAssignmentMutator) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.bitcodePaths = { fixtures::mutators_replace_assignment_module_bc_path(),
                                 fixtures::mutators_replace_assignment_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::NumberAssignConst>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program({}, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;
  std::unique_ptr<FunctionNameFilter> filter(new FunctionNameFilter("replace_assignment"));
  filters.functionFilters.push_back(filter.get());

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(size_t(1), result->getTests().size());
  ASSERT_EQ(size_t(2), result->getMutationResults().size());

  auto mutants = result->getMutationResults().begin();

  auto mutant1 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed, mutant1->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", mutant1->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant1->getExecutionResult().status);

  auto mutant2 = (mutants++)->get();
  ASSERT_EQ(ExecutionStatus::Passed, mutant2->getTest()->getExecutionResult().status);
  ASSERT_EQ("main", mutant2->getTest()->getTestName());
  ASSERT_EQ(ExecutionStatus::Failed, mutant2->getExecutionResult().status);

  ASSERT_EQ(mutants, result->getMutationResults().end());
}

TEST(Driver, customTest_withDynamicLibraries) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.dynamicLibraryPaths = { fixtures::dylibs_and_objects_distance_dylib_path() };
  configuration.bitcodePaths = { fixtures::dylibs_and_objects_test_bc_path(),
                                 fixtures::dylibs_and_objects_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::AddToSub>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader loader;
  Program program(
      configuration.dynamicLibraryPaths, {}, loader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;
  // filter.includeTest("passing");

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("main", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, customTest_withDynamicLibraries_and_ObjectFiles) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));

  configuration.dynamicLibraryPaths = { fixtures::dylibs_and_objects_distance_dylib_path() };
  configuration.objectFilePaths = { fixtures::dylibs_and_objects_test_o_path() };
  configuration.bitcodePaths = { fixtures::dylibs_and_objects_main_bc_path() };

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(std::make_unique<cxx::AddToSub>());
  MutationsFinder finder(std::move(mutators), configuration);

  BitcodeLoader bitcodeLoader;
  ObjectLoader objectLoader;
  Program program(configuration.dynamicLibraryPaths,
                  objectLoader.loadObjectFiles(configuration, diagnostics),
                  bitcodeLoader.loadBitcode(configuration, diagnostics));

  Toolchain toolchain(diagnostics, configuration);
  NullJunkDetector junkDetector;

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  ForkTimerSandbox sandbox;
  Filters filters;
  // filter.includeTest("passing");

  Driver driver(
      diagnostics, configuration, sandbox, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("main", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

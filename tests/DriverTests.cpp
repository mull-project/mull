#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/ExecutionResult.h"
#include "mull/JunkDetection/JunkDetector.h"
#include "mull/MutationsFinder.h"
#include "mull/ObjectLoader.h"
#include "mull/Program/Program.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/TestFrameworkFactory.h"
#include "mull/Toolchain/Toolchain.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>

#include <functional>
#include <utility>

#include <gtest/gtest.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLParser.h>
#include <mull/Diagnostics/Diagnostics.h>
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
  Filters filters;
  Driver driver(diagnostics, configuration, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(0u, result->getTests().size());
  ASSERT_EQ(0u, result->getMutationResults().size());
}

TEST(Driver, DISABLED_customTest_withDynamicLibraries) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));
  configuration.linker = "clang";
  configuration.skipSanityCheckRun = true;
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
  Filters filters;
  // filter.includeTest("passing");

  Driver driver(diagnostics, configuration, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("main", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

TEST(Driver, DISABLED_customTest_withDynamicLibraries_and_ObjectFiles) {
  Diagnostics diagnostics;
  Configuration configuration;

  configuration.customTests.push_back(mull::CustomTestDefinition("main", "main", "mull", {}));
  configuration.customTests.push_back(mull::CustomTestDefinition("main", "_main", "mull", {}));
  configuration.linker = "clang";
  configuration.skipSanityCheckRun = true;
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
  Filters filters;
  // filter.includeTest("passing");

  Driver driver(diagnostics, configuration, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(1U, result->getTests().size());

  auto &test = result->getTests().front();
  ASSERT_EQ(ExecutionStatus::Passed, test.getExecutionResult().status);
  ASSERT_EQ("main", test.getTestName());
  /// Could not find any mutations because there was no bitcode for testees
  ASSERT_EQ(0UL, result->getMutationResults().size());
}

#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/Configuration.h"
#include "mull/Driver.h"
#include "mull/MutationsFinder.h"
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

TEST(Driver, RunningWithNoTests) {
  Configuration configuration;
  Diagnostics diagnostics;
  MutationsFinder finder({}, configuration);
  Toolchain toolchain(diagnostics, configuration);

  TestFrameworkFactory testFrameworkFactory;
  TestFramework testFramework(
      testFrameworkFactory.customTestFramework(toolchain, configuration, diagnostics));
  Program program({});
  Filters filters;
  Driver driver(diagnostics, configuration, program, toolchain, filters, finder, testFramework);

  auto result = driver.run();
  ASSERT_EQ(0u, result->getTests().size());
  ASSERT_EQ(0u, result->getMutationResults().size());
}

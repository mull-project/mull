#include "mull/TestFrameworks/BoostTest/BoostTestFinder.h"
#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Program/Program.h"

#include <llvm/IR/LLVMContext.h>

#include <gtest/gtest.h>
#include <mull/Diagnostics/Diagnostics.h>

#include <memory>
#include <utility>
#include <vector>

using namespace mull;
using namespace llvm;

TEST(BoostTestFinder, FindTest) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::boosttest_test_boosttest_test_Test_bc_path(), context, diagnostics);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  Program program(std::move(bitcode));

  BoostTestFinder finder(diagnostics);

  auto tests = finder.findTests(program);

  ASSERT_EQ(1U, tests.size());

  ASSERT_EQ("SampleTestSuite/sampleTestCase", tests.at(0).getTestName());
  auto args = tests.at(0).getArguments();
  ASSERT_EQ(1, args.size());
  ASSERT_EQ("--run_test=SampleTestSuite/sampleTestCase", args.at(0));
}

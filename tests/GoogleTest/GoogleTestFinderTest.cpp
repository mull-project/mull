#include "mull/TestFrameworks/GoogleTest/GoogleTestFinder.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Program/Program.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/YAMLTraits.h>

#include <gtest/gtest.h>
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;
using namespace llvm;

#pragma mark - Finding Tests

TEST(GoogleTestFinder, FindTest) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeWithTests = loader.loadBitcodeAtPath(
      fixtures::google_test_google_test_Test_bc_path(), context, diagnostics);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeWithTests));
  Program program(std::move(bitcode));

  GoogleTestFinder finder;

  auto tests = finder.findTests(program);

  ASSERT_EQ(2U, tests.size());

  ASSERT_EQ("HelloTest.testSumOfTestee", tests[0].getTestName());
  ASSERT_EQ("HelloTest.testSumOfTestee2", tests[1].getTestName());
}

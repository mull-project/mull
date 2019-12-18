#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/BitcodeLoader.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/Test.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <mull/Diagnostics/Diagnostics.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(SimpleTestFinder, findTest) {
  Diagnostics diagnostics;
  LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeFile = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(), context, diagnostics);

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeFile));
  Program program({}, {}, std::move(bitcode));

  SimpleTestFinder finder;

  auto tests = finder.findTests(program);

  ASSERT_EQ(1U, tests.size());
}

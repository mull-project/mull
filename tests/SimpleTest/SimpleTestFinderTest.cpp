#include "mull/TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Filter.h"
#include "mull/ModuleLoader.h"
#include "mull/Program/Program.h"
#include "mull/TestFrameworks/Test.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(SimpleTestFinder, findTest) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto module = loader.loadModuleAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(module));
  Program program({}, {}, std::move(modules));

  Filter filter;
  SimpleTestFinder finder;

  auto tests = finder.findTests(program, filter);

  ASSERT_EQ(1U, tests.size());
}

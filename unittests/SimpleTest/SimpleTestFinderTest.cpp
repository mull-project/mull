#include "TestFrameworks/SimpleTest/SimpleTestFinder.h"
#include "TestFrameworks/SimpleTest/SimpleTest_Test.h"

#include "Context.h"
#include "Filter.h"
#include "TestModuleFactory.h"
#include "TestFrameworks/Test.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "gtest/gtest.h"
#include "FixturePaths.h"

using namespace mull;
using namespace llvm;

TEST(SimpleTestFinder, findTest) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto module = loader.loadModuleAtPath(fixtures::simple_test_count_letters_test_count_letters_bc_path(), llvmContext);

  Context context;
  context.addModule(std::move(module));

  Filter filter;
  SimpleTestFinder finder;

  auto tests = finder.findTests(context, filter);

  ASSERT_EQ(1U, tests.size());
}

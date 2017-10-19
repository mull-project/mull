#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "Filter.h"
#include "TestModuleFactory.h"
#include "Test.h"
#include "SimpleTest/SimpleTest_Test.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestFinder, FindTest) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto mullModuleWithTests = make_unique<MullModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mullModuleWithTests));

  Filter filter;
  SimpleTestFinder finder;

  auto tests = finder.findTests(Ctx, filter);

  ASSERT_EQ(1U, tests.size());
}

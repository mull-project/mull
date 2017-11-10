#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "Filter.h"
#include "TestModuleFactory.h"
#include "Test.h"
#include "SimpleTest/SimpleTest_Test.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(SimpleTestFinder, findTest) {
  auto module = TestModuleFactory.create_SimpleTest_CountLettersTest_Module();

  Context context;
  context.addModule(std::move(module));

  Filter filter;
  SimpleTestFinder finder;

  auto tests = finder.findTests(context, filter);

  ASSERT_EQ(1U, tests.size());
}

#include "SimpleTest/SimpleTestFinder.h"

#include "Context.h"
#include "Filter.h"
#include "Test.h"
#include "SimpleTest/SimpleTest_Test.h"

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "gtest/gtest.h"

#include "FixturesFactory.h"

using namespace mull;
using namespace llvm;

static LLVMContext globalContext;
static ModuleLoader loader(globalContext);
static FixturesFactory factory(loader);

TEST(SimpleTestFinder, findTest) {
  auto module = factory.create_simple_test_count_letters_test_count_letters_bc();

  Context context;
  context.addModule(std::move(module));

  Filter filter;
  SimpleTestFinder finder;

  auto tests = finder.findTests(context, filter);

  ASSERT_EQ(1U, tests.size());
}

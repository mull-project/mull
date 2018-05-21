#include "Context.h"

#include "TestModuleFactory.h"

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

TEST(Context, AddModule) {
  auto moduleWithTests = factory.create_simple_test_count_letters_test_count_letters_bc();

  Context Ctx;
  Ctx.addModule(std::move(moduleWithTests));

  ASSERT_EQ(1U, Ctx.getModules().size());
}

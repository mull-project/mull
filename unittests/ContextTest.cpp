#include "Context.h"

#include "TestModuleFactory.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(Context, AddModule) {
  auto moduleWithTests = TestModuleFactory.createSimpleTest_CountLettersTestModule();

  Context Ctx;
  Ctx.addModule(std::move(moduleWithTests));

  ASSERT_EQ(1U, Ctx.getModules().size());
}

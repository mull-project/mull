#include "Context.h"

#include "TestModuleFactory.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(Context, AddModule) {
  auto ModuleWithTests = TestModuleFactory.createTesterModule();
  auto mutangModuleWithTests = make_unique<MutangModule>(std::move(ModuleWithTests), "");

  Context Ctx;
  Ctx.addModule(std::move(mutangModuleWithTests));

  ASSERT_EQ(1U, Ctx.getModules().size());
}

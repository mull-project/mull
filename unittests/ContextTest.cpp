#include "Context.h"

#include "FixturePaths.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(Context, AddModule) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto module = loader.loadModuleAtPath(fixtures::simple_test_count_letters_test_count_letters_bc_path(), llvmContext);

  Context Ctx;
  Ctx.addModule(std::move(module));

  ASSERT_EQ(1U, Ctx.getModules().size());
}

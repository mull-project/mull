#include "Compiler.h"
#include "ModuleLoader.h"
#include "TestModuleFactory.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace Mutang;

static TestModuleFactory TestModuleFactory;

TEST(Compiler, CompileModule) {
  Compiler Compiler(false);

  auto SomeModule = TestModuleFactory.createTesterModule();
  MutangModule mutangModule(std::move(SomeModule), "");
  auto Binary = Compiler.compileModule(&mutangModule, "");

  ASSERT_NE(nullptr, Binary.getBinary());
}

#include "Compiler.h"

#include "TestModuleFactory.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace Mutang;

static TestModuleFactory TestModuleFactory;

TEST(Compiler, CompileModule) {
  Compiler Compiler;

  auto SomeModule = TestModuleFactory.createTesterModule();
  auto Binary = Compiler.compileModule(SomeModule.get());

  ASSERT_NE(nullptr, Binary.getBinary());
}

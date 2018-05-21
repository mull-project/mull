#include "Toolchain/Toolchain.h"
#include "ModuleLoader.h"
#include "TestModuleFactory.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>

#include "gtest/gtest.h"

#include "FixturesFactory.h"

using namespace llvm;
using namespace mull;

static LLVMContext globalContext;
static ModuleLoader loader(globalContext);
static FixturesFactory factory(loader);

TEST(Compiler, CompileModule) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(
                                  EngineBuilder().selectTarget(Triple(), "", "",
                                  SmallVector<std::string, 1>()));

  Compiler compiler(*targetMachine.get());

  auto module = factory.create_simple_test_count_letters_count_letters_bc();
  auto Binary = compiler.compileModule(module->getModule());

  ASSERT_NE(nullptr, Binary.getBinary());
}

#include "FixturePaths.h"
#include "mull/ModuleLoader.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>

#include "gtest/gtest.h"

using namespace llvm;
using namespace mull;

TEST(Compiler, CompileModule) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(EngineBuilder().selectTarget(
      Triple(), "", "", SmallVector<std::string, 1>()));

  Compiler compiler;

  LLVMContext llvmContext;
  ModuleLoader loader;
  auto module = loader.loadModuleAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(),
      llvmContext);
  auto binary = compiler.compileModule(module->getModule(), *targetMachine);

  ASSERT_NE(nullptr, binary.getBinary());
}

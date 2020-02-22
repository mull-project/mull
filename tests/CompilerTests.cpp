#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <mull/Diagnostics/Diagnostics.h>

#include "gtest/gtest.h"

using namespace llvm;
using namespace mull;

TEST(Compiler, CompileModule) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  std::unique_ptr<TargetMachine> targetMachine(
      EngineBuilder().selectTarget(Triple(), "", "", SmallVector<std::string, 1>()));

  Compiler compiler;
  Diagnostics diagnostics;
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcodeAtPath(
      fixtures::simple_test_count_letters_test_count_letters_bc_path(), llvmContext, diagnostics);
  auto binary = compiler.compileModule(bitcode->getModule(), *targetMachine);

  ASSERT_NE(nullptr, binary.getBinary());
}

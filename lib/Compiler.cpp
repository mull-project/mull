#include "Compiler.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;
using namespace Mutang;

Compiler::Compiler() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
}

object::OwningBinary<object::ObjectFile> Compiler::CompilerModule(Module *M) {
  assert(M);

  /// FIXME: Initialize everything once
  std::unique_ptr<TargetMachine> TM(
                              EngineBuilder().selectTarget(Triple(""), "", "",
                              SmallVector<std::string, 1>()));

  assert(TM && "Can't create TargetMachine");

  if (M->getDataLayout().isDefault()) {
    M->setDataLayout(TM->createDataLayout());
  }

  orc::SimpleCompiler Compiler(*TM);

  return Compiler(*M);
}

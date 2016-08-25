#include "Compiler.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"

#include "llvm/IR/DataLayout.h"

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
                              EngineBuilder().selectTarget(Triple("x86_64-apple-macosx10.11.0"), "", "",
                              SmallVector<std::string, 1>()));

  printf("%s\n", Triple("x86_64-apple-macosx10.11.0").str().c_str());
  printf("%s\n", TM->createDataLayout().getStringRepresentation().c_str());

  assert(TM && "Can't create TargetMachine");

  if (M->getDataLayout().isDefault()) {
    M->setDataLayout(TM->createDataLayout());
  }

  orc::SimpleCompiler Compiler(*TM);

  return Compiler(*M);
}

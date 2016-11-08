#include "Compiler.h"
#include "ModuleLoader.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include "llvm/IR/DataLayout.h"

using namespace llvm;
using namespace llvm::object;
using namespace Mutang;

Compiler::Compiler() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
}

OwningBinary<ObjectFile> Compiler::compileModule(const MutangModule &module) {
  return compileModule(module.getModule());
}

OwningBinary<ObjectFile> Compiler::compileModule(Module *module) {
  assert(module);

  /// FIXME: Initialize everything once
  std::unique_ptr<TargetMachine> targetMachine(
                              EngineBuilder().selectTarget(Triple(), "", "",
                              SmallVector<std::string, 1>()));

  assert(targetMachine && "Can't create TargetMachine");

  if (module->getDataLayout().isDefault()) {
    module->setDataLayout(targetMachine->createDataLayout());
  }

  orc::SimpleCompiler compiler(*targetMachine);

  OwningBinary<ObjectFile> objectFile = compiler(*module);

  return objectFile;
}

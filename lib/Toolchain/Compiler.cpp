#include "Toolchain/Compiler.h"

#include "MutangModule.h"

#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace llvm::object;
using namespace mull;

Compiler::Compiler(TargetMachine &machine) :
  targetMachine(machine)
{
}

OwningBinary<ObjectFile> Compiler::compileModule(const MutangModule &module) {
  return compileModule(module.getModule());
}

OwningBinary<ObjectFile> Compiler::compileModule(Module *module) {
  assert(module);

  if (module->getDataLayout().isDefault()) {
    module->setDataLayout(targetMachine.createDataLayout());
  }

  orc::SimpleCompiler compiler(targetMachine);

  OwningBinary<ObjectFile> objectFile = compiler(*module);

  return objectFile;
}

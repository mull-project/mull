#include "mull/Toolchain/Compiler.h"

#include "LLVMCompatibility.h"
#include "mull/MullModule.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/IR/Module.h>

using namespace llvm;
using namespace llvm::object;
using namespace mull;

OwningBinary<ObjectFile> Compiler::compileModule(const MullModule &module,
                                                 TargetMachine &machine) {
  return compileModule(module.getModule(), machine);
}

OwningBinary<ObjectFile> Compiler::compileModule(Module *module,
                                                 TargetMachine &machine) {
  assert(module);

  if (module->getDataLayout().isDefault()) {
    module->setDataLayout(machine.createDataLayout());
  }

  orc::SimpleCompiler compiler(machine);

  OwningBinary<ObjectFile> objectFile =
      llvm_compat::compileModule(compiler, *module);

  return objectFile;
}

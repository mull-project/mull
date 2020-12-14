#include "mull/Toolchain/Compiler.h"

#include "LLVMCompatibility.h"
#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;
using namespace llvm::object;
using namespace mull;

Compiler::Compiler(Diagnostics &diagnostics) : diagnostics(diagnostics) {}

OwningBinary<ObjectFile> Compiler::compileBitcode(const Bitcode &bitcode, TargetMachine &machine) {
  return compileModule(bitcode.getModule(), machine);
}

OwningBinary<ObjectFile> Compiler::compileModule(Module *module, TargetMachine &machine) {
  assert(module);

  std::string error;
  llvm::raw_string_ostream stream(error);
  if (llvm::verifyModule(*module, &stream)) {
    stream.flush();
    diagnostics.error(error);
  }

  if (module->getDataLayout().isDefault()) {
    module->setDataLayout(machine.createDataLayout());
  }

  orc::SimpleCompiler compiler(machine);

  OwningBinary<ObjectFile> objectFile = llvm_compat::compileModule(compiler, *module);

  return objectFile;
}

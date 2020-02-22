#include "mull/Toolchain/Toolchain.h"

#include "mull/Config/Configuration.h"

#include <llvm/ADT/Triple.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>

using namespace mull;

/// To make sure that initialization is getting called
/// before we create TargetMachine
/// Otherwise we cannot selectTarget, which lead us to invalid TargetMachine
Toolchain::NativeTarget::NativeTarget() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
}

Toolchain::Toolchain(Diagnostics &diagnostics, const Configuration &config)
    : nativeTarget(), machine(llvm::EngineBuilder().selectTarget(
                          llvm::Triple(), "", "", llvm::SmallVector<std::string, 1>())),
      objectCache(diagnostics, config.cacheEnabled, config.cacheDirectory), simpleCompiler(),
      nameMangler(machine->createDataLayout()) {}

ObjectCache &Toolchain::cache() {
  return objectCache;
}

Compiler &Toolchain::compiler() {
  return simpleCompiler;
}

llvm::TargetMachine &Toolchain::targetMachine() {
  return *machine;
}

mull::Mangler &Toolchain::mangler() {
  return nameMangler;
}

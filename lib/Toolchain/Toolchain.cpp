#include "Toolchain/Toolchain.h"
#include "Config/RawConfig.h"

#include <llvm/ADT/Triple.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <Toolchain/Toolchain.h>

using namespace mull;

/// To make sure that initialization is getting called
/// before we create TargetMachine
/// Otherwise we cannot selectTarget, which lead us to invalid TargetMachine
Toolchain::NativeTarget::NativeTarget() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
}

Toolchain::Toolchain(RawConfig &config) :
  nativeTarget(),
  machine(llvm::EngineBuilder().selectTarget(llvm::Triple(), "", "",
                                             llvm::SmallVector<std::string, 1>())),
  objectCache(config.cachingEnabled(), config.getCacheDirectory()),
  simpleCompiler(),
  nameMangler(machine->createDataLayout())
{
}

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


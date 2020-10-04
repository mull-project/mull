#include "mull/Toolchain/Resolvers/MutationResolver.h"

#include "mull/Toolchain/CXXRuntimeOverrides.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>

using namespace mull;
using namespace llvm;

MutationResolver::MutationResolver(CXXRuntimeOverrides &overrides, Trampolines &trampolines)
    : overrides(overrides), trampolines(trampolines) {}

llvm::JITSymbol MutationResolver::findSymbol(const std::string &name) {
  /// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return llvm::JITSymbol(address, JITSymbolFlags::Exported);
  }

  uint64_t *trampoline = trampolines.findTrampoline(name);
  if (trampoline != nullptr) {
    return llvm::JITSymbol((uint64_t)trampoline, JITSymbolFlags::Exported);
  }

  return llvm::JITSymbol(nullptr);
}

llvm::JITSymbol MutationResolver::findSymbolInLogicalDylib(const std::string &name) {
  return llvm::JITSymbol(nullptr);
}

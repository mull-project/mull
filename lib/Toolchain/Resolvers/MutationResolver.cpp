#include "mull/Toolchain/Resolvers/MutationResolver.h"

#include "mull/Toolchain/CXXRuntimeOverrides.h"
#include "mull/Toolchain/Trampolines.h"

#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>

using namespace mull;
using namespace llvm;

MutationResolver::MutationResolver(CXXRuntimeOverrides &overrides,
                                   Trampolines &trampolines, Mangler &mangler)
    : overrides(overrides), trampolines(trampolines), mangler(mangler) {}

llvm_compat::JITSymbolInfo
MutationResolver::findSymbol(const std::string &name) {
  /// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return llvm_compat::JITSymbolInfo(address, JITSymbolFlags::Exported);
  }

  uint64_t *trampoline = trampolines.findTrampoline(name);
  if (trampoline != nullptr) {
    return llvm_compat::JITSymbolInfo((uint64_t)trampoline,
                                      JITSymbolFlags::Exported);
  }

  return llvm_compat::JITSymbolInfo(nullptr);
}

llvm_compat::JITSymbolInfo
MutationResolver::findSymbolInLogicalDylib(const std::string &name) {
  return llvm_compat::JITSymbolInfo(nullptr);
}

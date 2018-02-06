#include "Toolchain/Resolvers/NativeResolver.h"

#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

using namespace mull;
using namespace llvm;

NativeResolver::NativeResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides)
: overrides(overrides) {}

llvm::RuntimeDyld::SymbolInfo NativeResolver::findSymbol(const std::string &name) {
  /// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return RuntimeDyld::SymbolInfo(address, JITSymbolFlags::Exported);
  }

  return RuntimeDyld::SymbolInfo(nullptr);
}

llvm::RuntimeDyld::SymbolInfo NativeResolver::findSymbolInLogicalDylib(const std::string &name) {
  return RuntimeDyld::SymbolInfo(nullptr);
}

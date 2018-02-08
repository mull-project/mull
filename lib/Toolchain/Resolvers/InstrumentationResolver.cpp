#include "Toolchain/Resolvers/InstrumentationResolver.h"

#include "Instrumentation/Instrumentation.h"
#include "Mangler.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>

using namespace mull;
using namespace llvm;

InstrumentationResolver::InstrumentationResolver(orc::LocalCXXRuntimeOverrides &overrides,
                                                 Instrumentation &instrumentation,
                                                 Mangler &mangler,
                                                 InstrumentationInfo **trampoline)
: overrides(overrides),
instrumentation(instrumentation),
instrumentationInfoName(mangler.getNameWithPrefix(instrumentation.instrumentationInfoVariableName())),
functionOffsetPrefix(mangler.getNameWithPrefix(instrumentation.functionIndexOffsetPrefix())),
trampoline(trampoline) {}

RuntimeDyld::SymbolInfo InstrumentationResolver::findSymbol(const std::string &name) {
  /// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return RuntimeDyld::SymbolInfo(address, JITSymbolFlags::Exported);
  }

  if (name == instrumentationInfoName) {
    return RuntimeDyld::SymbolInfo((uint64_t)trampoline, JITSymbolFlags::Exported);
  }

  if (name.find(functionOffsetPrefix) != std::string::npos) {
    auto moduleName = name.substr(functionOffsetPrefix.length());
    auto &mapping = instrumentation.getFunctionOffsetMapping();
    return RuntimeDyld::SymbolInfo((uint64_t)&mapping[moduleName], JITSymbolFlags::Exported);
  }

  return RuntimeDyld::SymbolInfo(nullptr);
}

RuntimeDyld::SymbolInfo InstrumentationResolver::findSymbolInLogicalDylib(const std::string &name) {
  return RuntimeDyld::SymbolInfo(nullptr);
}

#include "Toolchain/Resolvers/InstrumentationResolver.h"

#include "Instrumentation/Instrumentation.h"
#include "Toolchain/Mangler.h"

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

llvm_compat::JITSymbolInfo InstrumentationResolver::findSymbol(const std::string &name) {
  /// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return llvm_compat::JITSymbolInfo(address, JITSymbolFlags::Exported);
  }

  if (name == instrumentationInfoName) {
    return llvm_compat::JITSymbolInfo((uint64_t)trampoline, JITSymbolFlags::Exported);
  }

  if (name.find(functionOffsetPrefix) != std::string::npos) {
    auto moduleName = name.substr(functionOffsetPrefix.length());
    auto &mapping = instrumentation.getFunctionOffsetMapping();
    return llvm_compat::JITSymbolInfo((uint64_t)&mapping[moduleName], JITSymbolFlags::Exported);
  }

  return llvm_compat::JITSymbolInfo(nullptr);
}

llvm_compat::JITSymbolInfo InstrumentationResolver::findSymbolInLogicalDylib(const std::string &name) {
  return llvm_compat::JITSymbolInfo(nullptr);
}

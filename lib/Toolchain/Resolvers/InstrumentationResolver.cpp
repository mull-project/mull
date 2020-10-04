#include "mull/Toolchain/Resolvers/InstrumentationResolver.h"

#include "mull/Instrumentation/Instrumentation.h"
#include "mull/Toolchain/CXXRuntimeOverrides.h"
#include "mull/Toolchain/Mangler.h"

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>

using namespace mull;
using namespace llvm;

InstrumentationResolver::InstrumentationResolver(
    CXXRuntimeOverrides &overrides, Instrumentation &instrumentation,
    Mangler &mangler, InstrumentationInfo **trampoline)
    : overrides(overrides), instrumentation(instrumentation),
      instrumentationInfoName(mangler.getNameWithPrefix(
          instrumentation.instrumentationInfoVariableName())),
      functionOffsetPrefix(mangler.getNameWithPrefix(
          instrumentation.functionIndexOffsetPrefix())),
      trampoline(trampoline) {}

llvm::JITSymbol InstrumentationResolver::findSymbol(const std::string &name) {
  /// Overrides should go first, otherwise functions of the host process
  /// will take over and crash the system later
  if (auto symbol = overrides.searchOverrides(name)) {
    return symbol;
  }

  if (auto address = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
    return llvm::JITSymbol(address, JITSymbolFlags::Exported);
  }

  if (name == instrumentationInfoName) {
    return llvm::JITSymbol((uint64_t)trampoline, JITSymbolFlags::Exported);
  }

  if (name.find(functionOffsetPrefix) != std::string::npos) {
    auto moduleName = name.substr(functionOffsetPrefix.length());
    auto &mapping = instrumentation.getFunctionOffsetMapping();
    return llvm::JITSymbol((uint64_t)&mapping[moduleName], JITSymbolFlags::Exported);
  }

  return llvm::JITSymbol(nullptr);
}

llvm::JITSymbol InstrumentationResolver::findSymbolInLogicalDylib(const std::string &name) {
  return llvm::JITSymbol(nullptr);
}

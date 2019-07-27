#pragma once

#include "LLVMCompatibility.h"
#include <string>

namespace mull {

class Mangler;
class Instrumentation;
class CXXRuntimeOverrides;
struct InstrumentationInfo;

class InstrumentationResolver : public llvm_compat::SymbolResolver {
  CXXRuntimeOverrides &overrides;
  Instrumentation &instrumentation;
  std::string instrumentationInfoName;
  std::string functionOffsetPrefix;
  InstrumentationInfo **trampoline;

public:
  InstrumentationResolver(CXXRuntimeOverrides &overrides,
                          Instrumentation &instrumentation,
                          mull::Mangler &mangler,
                          InstrumentationInfo **trampoline);

  llvm_compat::JITSymbolInfo findSymbol(const std::string &name) override;
  llvm_compat::JITSymbolInfo
  findSymbolInLogicalDylib(const std::string &name) override;
};
} // namespace mull

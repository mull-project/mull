#pragma once

#include "LLVMCompatibility.h"
#include <string>

namespace llvm {
namespace orc {
class LocalCXXRuntimeOverrides;
}
} // namespace llvm

namespace mull {

class Mangler;
class Instrumentation;
struct InstrumentationInfo;

class InstrumentationResolver : public llvm_compat::SymbolResolver {
  llvm_compat::CXXRuntimeOverrides &overrides;
  Instrumentation &instrumentation;
  std::string instrumentationInfoName;
  std::string functionOffsetPrefix;
  InstrumentationInfo **trampoline;

public:
  InstrumentationResolver(llvm_compat::CXXRuntimeOverrides &overrides,
                          Instrumentation &instrumentation,
                          mull::Mangler &mangler,
                          InstrumentationInfo **trampoline);

  llvm_compat::JITSymbolInfo findSymbol(const std::string &name) override;
  llvm_compat::JITSymbolInfo
  findSymbolInLogicalDylib(const std::string &name) override;
};
} // namespace mull

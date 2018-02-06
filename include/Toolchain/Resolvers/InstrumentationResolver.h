#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <string>

namespace llvm {
namespace orc {
  class LocalCXXRuntimeOverrides;
}
}

namespace mull {

class Mangler;
class Instrumentation;
struct InstrumentationInfo;

class InstrumentationResolver : public llvm::RuntimeDyld::SymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
  std::string instrumentationInfoName;
  InstrumentationInfo **trampoline;
public:
  InstrumentationResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides,
                          Instrumentation &instrumentation,
                          mull::Mangler &mangler,
                          InstrumentationInfo **trampoline);

  llvm::RuntimeDyld::SymbolInfo findSymbol(const std::string &name) override;
  llvm::RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &name) override;
};
}

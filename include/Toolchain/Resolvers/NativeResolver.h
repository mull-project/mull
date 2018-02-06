#pragma once

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>

namespace mull {
class NativeResolver : public llvm::RuntimeDyld::SymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
public:
  NativeResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides);
  llvm::RuntimeDyld::SymbolInfo findSymbol(const std::string &name) override;
  llvm::RuntimeDyld::SymbolInfo findSymbolInLogicalDylib(const std::string &name) override;
};
}

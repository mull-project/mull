#pragma once

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include "LLVMCompatibility.h"

namespace mull {
class NativeResolver : public llvm_compat::SymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
public:
  NativeResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides);
  llvm_compat::JITSymbolInfo findSymbol(const std::string &name) override;
  llvm_compat::JITSymbolInfo findSymbolInLogicalDylib(const std::string &name) override;
};
}

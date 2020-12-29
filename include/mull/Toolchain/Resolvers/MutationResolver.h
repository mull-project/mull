#pragma once

#include "LLVMCompatibility.h"
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

namespace mull {
class CXXRuntimeOverrides;

class MutationResolver : public llvm_compat::SymbolResolver {
public:
  explicit MutationResolver(CXXRuntimeOverrides &overrides);
  llvm::JITSymbol findSymbol(const std::string &name) override;
  llvm::JITSymbol findSymbolInLogicalDylib(const std::string &name) override;

private:
  CXXRuntimeOverrides &overrides;
};
} // namespace mull

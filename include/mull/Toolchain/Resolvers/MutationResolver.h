#pragma once

#include "LLVMCompatibility.h"
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

namespace mull {
class Trampolines;
class CXXRuntimeOverrides;

class MutationResolver : public llvm_compat::SymbolResolver {
public:
  MutationResolver(CXXRuntimeOverrides &overrides, Trampolines &trampolines);
  llvm::JITSymbol findSymbol(const std::string &name) override;
  llvm::JITSymbol findSymbolInLogicalDylib(const std::string &name) override;

private:
  CXXRuntimeOverrides &overrides;
  Trampolines &trampolines;
};
} // namespace mull

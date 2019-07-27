#pragma once

#include "LLVMCompatibility.h"
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>

namespace mull {
class Trampolines;
class Mangler;
class CXXRuntimeOverrides;

class MutationResolver : public llvm_compat::SymbolResolver {
  CXXRuntimeOverrides &overrides;
  Trampolines &trampolines;
  Mangler &mangler;

public:
  MutationResolver(CXXRuntimeOverrides &overrides, Trampolines &trampolines,
                   Mangler &mangler);
  llvm_compat::JITSymbolInfo findSymbol(const std::string &name) override;
  llvm_compat::JITSymbolInfo
  findSymbolInLogicalDylib(const std::string &name) override;
};
} // namespace mull

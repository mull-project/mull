#pragma once

#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include "LLVMCompatibility.h"

namespace mull {
class Trampolines;
class Mangler;

class MutationResolver : public llvm_compat::SymbolResolver {
  llvm::orc::LocalCXXRuntimeOverrides &overrides;
  Trampolines &trampolines;
  Mangler &mangler;
public:
  MutationResolver(llvm::orc::LocalCXXRuntimeOverrides &overrides, Trampolines &trampolines, Mangler &mangler);
  llvm_compat::JITSymbolInfo findSymbol(const std::string &name) override;
  llvm_compat::JITSymbolInfo findSymbolInLogicalDylib(const std::string &name) override;
};
}

#include "LLVMCompatibility.h"
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/Function.h>

using namespace llvm;

namespace llvm_compat {

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  return symbol.getAddress();
}

Function *cloneFunction(const Function *function) {
  ValueToValueMapTy map;
  /// Newer versions of LLVM do not have the third boolean argument
  return CloneFunction(function, map, true);
}

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  /// Back-ported from a newer version of LLVM
  JITSymbolFlags Flags = JITSymbolFlags::None;
  if (symbol.getFlags() & object::BasicSymbolRef::SF_Weak)
    Flags |= JITSymbolFlags::Weak;
  if (symbol.getFlags() & object::BasicSymbolRef::SF_Exported)
    Flags |= JITSymbolFlags::Exported;
  return Flags;
}

std::string moduleSourceFile(llvm::Module &module) {
//  instruction->getModule()->getSourceFileName()

  auto dbgCU = module.getNamedMetadata("llvm.dbg.cu");
  if (dbgCU == nullptr) {
    return std::string();
  }

  for (int i = 0; i < dbgCU->getNumOperands(); i++) {
    auto compileUnit = dyn_cast<DICompileUnit>(dbgCU->getOperand(i));
    if (!compileUnit) {
      return std::string();
    }

    for (int j = 0; j < compileUnit->getNumOperands(); j++) {
      auto file = dyn_cast<DIFile>(compileUnit->getOperand(j).get());
      if (!file) {
        return std::string();
      }

      return file->getFilename();
    }
  }

  return std::string();
}

}


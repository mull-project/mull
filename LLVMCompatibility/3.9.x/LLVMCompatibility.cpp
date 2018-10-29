#include "LLVMCompatibility.h"
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/Function.h>

using namespace llvm;

namespace llvm_compat {

Function *cloneFunction(Function *function) {
  ValueToValueMapTy map;
  return CloneFunction(function, map);
}

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  return symbol.getAddress();
}

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return orc::JITSymbol::flagsFromObjectSymbol(symbol);
}

std::string moduleSourceFile(llvm::Module &module) {
  return module.getSourceFileName();
}

}


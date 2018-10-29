#include "LLVMCompatibility.h"
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/Function.h>

using namespace llvm;

namespace llvm_compat {

Function *cloneFunction(Function *function) {
  ValueToValueMapTy map;
  return CloneFunction(function, map);
}

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return JITSymbolFlags::fromObjectSymbol(symbol);
}

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  auto addressOrError = symbol.getAddress();
  if (!addressOrError) {
    consumeError(addressOrError.takeError());
    return 0;
  }

  return addressOrError.get();
}

std::string moduleSourceFile(llvm::Module &module) {
  return module.getSourceFileName();
}

}


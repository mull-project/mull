#include "LLVMCompatibility.h"

using namespace llvm;

namespace llvm_compat {

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  return symbol.getAddress();
}

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return JITSymbolFlags::fromObjectSymbol(symbol);
}

std::string moduleSourceFile(llvm::Module &module) {
  return module.getSourceFileName();
}

}


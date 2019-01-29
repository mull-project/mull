#include "LLVMCompatibility.h"

using namespace llvm;

namespace llvm_compat {

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  return symbol.getAddress();
}

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return JITSymbolFlags::fromObjectSymbol(symbol);
}

object::OwningBinary<object::ObjectFile>
compileModule(orc::SimpleCompiler &compiler, llvm::Module &module) {
  auto objectFile = compiler(module);
  return std::move(objectFile);
}

}


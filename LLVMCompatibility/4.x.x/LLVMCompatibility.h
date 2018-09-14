#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/Bitcode/BitcodeReader.h>

namespace llvm_compat {
  using namespace llvm;

  typedef JITSymbolResolver SymbolResolver;
  typedef JITSymbol JITSymbolInfo;
  typedef JITSymbol JITSymbol;

  uint64_t JITSymbolAddress(JITSymbol &symbol);
  JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);
}


#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>

namespace llvm_compat {
  using namespace llvm;

  typedef LegacyJITSymbolResolver SymbolResolver;
  typedef JITSymbol JITSymbolInfo;
  typedef JITSymbol JITSymbol;

  uint64_t JITSymbolAddress(JITSymbol &symbol);
  JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);
  object::OwningBinary<object::ObjectFile>
  compileModule(orc::SimpleCompiler &compiler, llvm::Module &module);
}


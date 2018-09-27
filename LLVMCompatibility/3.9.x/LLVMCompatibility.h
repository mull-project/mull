#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/Bitcode/ReaderWriter.h>

namespace llvm_compat {
  using namespace llvm;

  typedef RuntimeDyld::SymbolResolver SymbolResolver;
  typedef RuntimeDyld::SymbolInfo JITSymbolInfo;
  typedef orc::JITSymbol JITSymbol;

  uint64_t JITSymbolAddress(JITSymbol &symbol);
  JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);
}


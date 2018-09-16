#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/Bitcode/BitcodeReader.h>

namespace llvm_compat {
using namespace llvm;

typedef JITSymbolResolver SymbolResolver;
typedef JITSymbol JITSymbolInfo;
typedef JITSymbol JITSymbol;

template<class T>
using Expected = Expected<T>;

template<class T>
void ignoreError(Expected<T> &e) { consumeError(e.takeError()); }

std::string moduleSourceFile(llvm::Module &module);

uint64_t JITSymbolAddress(JITSymbol &symbol);
JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);
}


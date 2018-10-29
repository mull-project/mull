#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/Bitcode/ReaderWriter.h>

namespace llvm {
class Module;
class Function;
}

namespace llvm_compat {
using namespace llvm;

typedef RuntimeDyld::SymbolResolver SymbolResolver;
typedef RuntimeDyld::SymbolInfo JITSymbolInfo;
typedef orc::JITSymbol JITSymbol;

template<class T>
using Expected = Expected<T>;

template<class T>
void ignoreError(Expected<T> &e) { consumeError(e.takeError()); }

std::string moduleSourceFile(llvm::Module &module);

uint64_t JITSymbolAddress(JITSymbol &symbol);
JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);
llvm::Function *cloneFunction(llvm::Function *function);
}

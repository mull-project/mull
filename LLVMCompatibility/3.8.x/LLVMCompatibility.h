#pragma once

#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/ExecutionEngine/Orc/JITSymbol.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/ErrorOr.h>

namespace llvm {
class Module;
class Function;
}

namespace llvm_compat {
using namespace llvm;

typedef RuntimeDyld::SymbolResolver SymbolResolver;
typedef RuntimeDyld::SymbolInfo JITSymbolInfo;
typedef RuntimeDyld::SymbolInfo JITSymbol;

template<class T>
using Expected = ErrorOr<T>;

template<class T>
void ignoreError(Expected<T> &e) {}

uint64_t JITSymbolAddress(JITSymbol &symbol);
JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);
std::string moduleSourceFile(llvm::Module &module);
llvm::Function *cloneFunction(const llvm::Function *function);
}


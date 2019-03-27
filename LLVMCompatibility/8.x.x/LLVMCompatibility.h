#pragma once

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>

namespace llvm {
class raw_ostream;
}

namespace llvm_compat {
using namespace llvm;

typedef LegacyJITSymbolResolver SymbolResolver;
typedef JITSymbol JITSymbolInfo;
typedef JITSymbol JITSymbol;
typedef orc::LegacyLocalCXXRuntimeOverrides CXXRuntimeOverrides;

uint64_t JITSymbolAddress(JITSymbol &symbol);

JITSymbolFlags
JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);

object::OwningBinary<object::ObjectFile>
compileModule(orc::SimpleCompiler &compiler, llvm::Module &module);

std::unique_ptr<Module> parseBitcode(MemoryBufferRef bufferRef,
                                     LLVMContext &context);

void setVersionPrinter(void (*oldPrinter)(), void (*newPrinter)(raw_ostream &));

} // namespace llvm_compat

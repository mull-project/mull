#pragma once

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>

namespace llvm {
class DICompileUnit;
class DebugLoc;
namespace object {
class SectionRef;
}
} // namespace llvm

namespace llvm_compat {
using namespace llvm;

typedef LegacyJITSymbolResolver SymbolResolver;

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol);

object::OwningBinary<object::ObjectFile> compileModule(orc::SimpleCompiler &compiler,
                                                       llvm::Module &module);

StringRef getSectionContent(const object::SectionRef &section);
StringRef getSectionName(const object::SectionRef &section);
DICompileUnit *getUnit(const DebugLoc &debugLocation);
} // namespace llvm_compat

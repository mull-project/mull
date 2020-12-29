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
namespace legacy {
class PassManagerBase;
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

std::string demangle(const std::string &MangledName);

object::BasicSymbolRef::Flags flagsFromSymbol(object::BasicSymbolRef &symbol);
llvm::Value *getOrInsertFunction(llvm::Module *module, StringRef name, FunctionType *type);
void writeBitcodeToFile(const llvm::Module &module, llvm::raw_fd_ostream &stream);
bool addPassesToEmitObjectFile(TargetMachine *targetMachine, legacy::PassManagerBase &passManager,
                               raw_pwrite_stream &out);
} // namespace llvm_compat

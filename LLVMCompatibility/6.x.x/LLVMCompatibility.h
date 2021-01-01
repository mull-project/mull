#pragma once

#include <llvm/ADT/StringRef.h>

namespace llvm {

class DICompileUnit;
class DebugLoc;
class TargetMachine;
class FunctionType;
class Value;
class Module;
class raw_pwrite_stream;
class raw_fd_ostream;

namespace object {
class SectionRef;
}

namespace legacy {
class PassManagerBase;
}
} // namespace llvm

namespace llvm_compat {
llvm::StringRef getSectionContent(const llvm::object::SectionRef &section);
llvm::StringRef getSectionName(const llvm::object::SectionRef &section);
llvm::DICompileUnit *getUnit(const llvm::DebugLoc &debugLocation);
std::string demangle(const std::string &MangledName);
llvm::Value *getOrInsertFunction(llvm::Module *module, llvm::StringRef name,
                                 llvm::FunctionType *type);
void writeBitcodeToFile(const llvm::Module &module, llvm::raw_fd_ostream &stream);
bool addPassesToEmitObjectFile(llvm::TargetMachine *targetMachine,
                               llvm::legacy::PassManagerBase &passManager,
                               llvm::raw_pwrite_stream &out);
} // namespace llvm_compat

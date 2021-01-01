#include "LLVMCompatibility.h"

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Module.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Target/TargetMachine.h>

using namespace llvm;

namespace llvm_compat {

StringRef getSectionContent(const object::SectionRef &section) {
  StringRef content;
  section.getContents(content);
  return content;
}
StringRef getSectionName(const object::SectionRef &section) {
  StringRef name;
  section.getName(name);
  return name;
}

DICompileUnit *getUnit(const DebugLoc &debugLocation) {
  DIScope *scope = debugLocation->getScope();
  while (!llvm::isa<llvm::DISubprogram>(scope) && scope != nullptr) {
    scope = scope->getScope().resolve();
  }
  return scope ? llvm::cast<llvm::DISubprogram>(scope)->getUnit() : nullptr;
}

static bool isItaniumEncoding(const std::string &MangledName) {
  size_t Pos = MangledName.find_first_not_of('_');
  // A valid Itanium encoding requires 1-4 leading underscores, followed by 'Z'.
  return Pos > 0 && Pos <= 4 && MangledName[Pos] == 'Z';
}

std::string demangle(const std::string &MangledName) {
  char *Demangled;
  if (isItaniumEncoding(MangledName))
    Demangled = llvm::itaniumDemangle(MangledName.c_str(), nullptr, nullptr, nullptr);
  else
    Demangled = llvm::microsoftDemangle(MangledName.c_str(), nullptr, nullptr, nullptr);

  if (!Demangled)
    return MangledName;

  std::string Ret = Demangled;
  std::free(Demangled);
  return Ret;
}

llvm::Value *getOrInsertFunction(llvm::Module *module, StringRef name, FunctionType *type) {
  return module->getOrInsertFunction(name, type);
}

void writeBitcodeToFile(const llvm::Module &module, llvm::raw_fd_ostream &stream) {
  llvm::WriteBitcodeToFile(module, stream);
}

bool addPassesToEmitObjectFile(TargetMachine *targetMachine, legacy::PassManagerBase &passManager,
                               raw_pwrite_stream &out) {
  return targetMachine->addPassesToEmitFile(
      passManager, out, nullptr, llvm::TargetMachine::CGFT_ObjectFile);
}

} // namespace llvm_compat

#include "LLVMCompatibility.h"

#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/Object/ObjectFile.h>

using namespace llvm;

namespace llvm_compat {

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return JITSymbolFlags::fromObjectSymbol(symbol);
}

object::OwningBinary<object::ObjectFile> compileModule(orc::SimpleCompiler &compiler,
                                                       llvm::Module &module) {
  return compiler(module);
}

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

std::string demangle(const std::string &MangledName) {
  char *Demangled = llvm::itaniumDemangle(MangledName.c_str(), nullptr, nullptr, nullptr);

  if (!Demangled)
    return MangledName;

  std::string Ret = Demangled;
  std::free(Demangled);
  return Ret;
}

object::BasicSymbolRef::Flags flagsFromSymbol(object::BasicSymbolRef &symbol) {
  return static_cast<object::BasicSymbolRef::Flags>(symbol.getFlags());
}

} // namespace llvm_compat

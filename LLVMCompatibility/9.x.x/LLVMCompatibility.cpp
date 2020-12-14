#include "LLVMCompatibility.h"

#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/Object/ObjectFile.h>

using namespace llvm;

namespace llvm_compat {

JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  auto symbolOrError = JITSymbolFlags::fromObjectSymbol(symbol);
  if (symbolOrError) {
    return symbolOrError.get();
  }
  return JITSymbolFlags();
}

object::OwningBinary<object::ObjectFile> compileModule(orc::SimpleCompiler &compiler,
                                                       llvm::Module &module) {
  auto buffer = compiler(module);
  auto bufferRef = buffer->getMemBufferRef();
  auto objectOrError = object::ObjectFile::createObjectFile(bufferRef);
  if (!objectOrError) {
    consumeError(objectOrError.takeError());
    return object::OwningBinary<object::ObjectFile>();
  }

  return object::OwningBinary<object::ObjectFile>(std::move(objectOrError.get()),
                                                  std::move(buffer));
}

StringRef getSectionContent(const object::SectionRef &section) {
  Expected<StringRef> content = section.getContents();
  if (!content) {
    return {};
  }
  return content.get();
}
StringRef getSectionName(const object::SectionRef &section) {
  StringRef name;
  section.getName(name);
  return name;
}

DICompileUnit *getUnit(const DebugLoc &debugLocation) {
  DIScope *scope = debugLocation->getScope();
  while (!llvm::isa<llvm::DISubprogram>(scope) && scope != nullptr) {
    scope = scope->getScope();
  }
  return scope ? llvm::cast<llvm::DISubprogram>(scope)->getUnit() : nullptr;
}

std::string demangle(const std::string &MangledName) {
  return llvm::demangle(MangledName);
}

object::BasicSymbolRef::Flags flagsFromSymbol(object::BasicSymbolRef &symbol) {
  return static_cast<object::BasicSymbolRef::Flags>(symbol.getFlags());
}

} // namespace llvm_compat

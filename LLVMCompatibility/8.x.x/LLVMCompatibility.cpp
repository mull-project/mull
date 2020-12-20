#include "LLVMCompatibility.h"

#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Module.h>
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

object::BasicSymbolRef::Flags flagsFromSymbol(object::BasicSymbolRef &symbol) {
  return static_cast<object::BasicSymbolRef::Flags>(symbol.getFlags());
}

llvm::Value *getOrInsertFunction(llvm::Module *module, StringRef name, FunctionType *type) {
  return module->getOrInsertFunction(name, type);
}

} // namespace llvm_compat

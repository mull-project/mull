#include "LLVMCompatibility.h"

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/Object/ObjectFile.h>

using namespace llvm;

namespace llvm_compat {

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  return symbol.getAddress();
}


JITSymbolFlags JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return orc::JITSymbol::flagsFromObjectSymbol(symbol);
}

object::OwningBinary<object::ObjectFile> compileModule(orc::SimpleCompiler &compiler,
                                                       llvm::Module &module) {
  return compiler(module);
}

std::unique_ptr<Module> parseBitcode(MemoryBufferRef bufferRef, LLVMContext &context) {
  auto module = parseBitcodeFile(bufferRef, context);
  if (!module) {
    std::string message("\nparseBitcodeFile failed: ");
    message += module.getError().message() + "\n";
    llvm::errs() << message;
    return std::unique_ptr<Module>();
  }

  return std::move(module.get());
}

void setVersionPrinter(void (*oldPrinter)(), void (*newPrinter)(raw_ostream &)) {
  llvm::cl::SetVersionPrinter(oldPrinter);
}

StringRef getSectionContent(const object::SectionRef &section) {
  StringRef content;
  section.getContents(content);
  return content;
}

DICompileUnit *getUnit(const DebugLoc &debugLocation) {
  DIScope *scope = debugLocation->getScope();
  while (!llvm::isa<llvm::DISubprogram>(scope) && scope != nullptr) {
    scope = scope->getScope().resolve();
  }
  return scope ? llvm::cast<llvm::DISubprogram>(scope)->getUnit() : nullptr;
}

bool parseCommandLineOptions(int argc, char **argv) {
  return llvm::cl::ParseCommandLineOptions(argc, argv, "", true);
}

} // namespace llvm_compat

#include "LLVMCompatibility.h"

#include <llvm/Bitcode/ReaderWriter.h>

using namespace llvm;

namespace llvm_compat {

uint64_t JITSymbolAddress(JITSymbol &symbol) { return symbol.getAddress(); }

JITSymbolFlags
JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  return orc::JITSymbol::flagsFromObjectSymbol(symbol);
}

object::OwningBinary<object::ObjectFile>
compileModule(orc::SimpleCompiler &compiler, llvm::Module &module) {
  auto objectFile = compiler(module);
  return std::move(objectFile);
}

std::unique_ptr<Module> parseBitcode(MemoryBufferRef bufferRef,
                                     LLVMContext &context) {
  auto module = parseBitcodeFile(bufferRef, context);
  if (!module) {
    std::string message("\nparseBitcodeFile failed: ");
    message += module.getError().message() + "\n";
    llvm::errs() << message;
    return std::unique_ptr<Module>();
  }

  return std::move(module.get());
}

} // namespace llvm_compat

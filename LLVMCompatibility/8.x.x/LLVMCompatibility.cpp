#include "LLVMCompatibility.h"

#include <llvm/Bitcode/BitcodeReader.h>

using namespace llvm;

namespace llvm_compat {

JITSymbolFlags
JITSymbolFlagsFromObjectSymbol(const object::BasicSymbolRef &symbol) {
  auto symbolOrError = JITSymbolFlags::fromObjectSymbol(symbol);
  if (symbolOrError) {
    return symbolOrError.get();
  }
  return JITSymbolFlags();
}

uint64_t JITSymbolAddress(JITSymbol &symbol) {
  auto addressOrError = symbol.getAddress();
  if (!addressOrError) {
    consumeError(addressOrError.takeError());
    return 0;
  }

  return addressOrError.get();
}

object::OwningBinary<object::ObjectFile>
compileModule(orc::SimpleCompiler &compiler, llvm::Module &module) {
  auto buffer = compiler(module);
  auto bufferRef = buffer->getMemBufferRef();
  auto objectOrError = object::ObjectFile::createObjectFile(bufferRef);
  if (!objectOrError) {
    consumeError(objectOrError.takeError());
    return object::OwningBinary<object::ObjectFile>();
  }

  return object::OwningBinary<object::ObjectFile>(
      std::move(objectOrError.get()), std::move(buffer));
}

std::unique_ptr<Module> parseBitcode(MemoryBufferRef bufferRef,
                                     LLVMContext &context) {
  auto module = parseBitcodeFile(bufferRef, context);
  if (!module) {
    logAllUnhandledErrors(module.takeError(), errs(),
                          "\nparseBitcodeFile failed: ");
    return std::unique_ptr<Module>();
  }

  return std::move(module.get());
}

} // namespace llvm_compat

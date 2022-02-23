#include "BitcodeLoader.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <sstream>

using namespace llvm;
using namespace mull;

std::unique_ptr<Module> mull::parseBitcode(MemoryBufferRef bufferRef, LLVMContext &context,
                                           Diagnostics &diagnostics) {
  auto module = parseBitcodeFile(bufferRef, context);
  if (!module) {
    std::stringstream errorMessage;
    errorMessage << "parseBitcodeFile failed: \"" << toString(module.takeError()) << "\".";

    auto producerString = getBitcodeProducerString(bufferRef);
    if (producerString) {
      errorMessage << " ";
      errorMessage << "The bitcode file was created with LLVM version: " << producerString.get();
    }

    diagnostics.warning(errorMessage.str());

    return std::unique_ptr<Module>();
  }

  return std::move(module.get());
}

std::unique_ptr<Module> mull::loadModuleFromBuffer(LLVMContext &context, MemoryBuffer &buffer,
                                                   Diagnostics &diagnostics) {
  return parseBitcode(buffer.getMemBufferRef(), context, diagnostics);
}

std::unique_ptr<Bitcode> BitcodeLoader::loadBitcodeAtPath(const std::string &path,
                                                          Diagnostics &diagnostics) {
  auto buffer = MemoryBuffer::getFile(path);
  if (!buffer) {
    std::stringstream message;
    message << "Cannot load module " << path << '\n';
    diagnostics.error(message.str());
    return nullptr;
  }

  auto context = std::make_unique<LLVMContext>();
  MemoryBuffer *b = buffer.get().get();
  std::unique_ptr<llvm::Module> module(loadModuleFromBuffer(*context, *b, diagnostics));

  if (module == nullptr) {
    std::stringstream message;
    message << "Cannot parse bitcode " << path << '\n';
    diagnostics.error(message.str());
    return nullptr;
  }

  return std::make_unique<Bitcode>(std::move(context), std::move(module));
}

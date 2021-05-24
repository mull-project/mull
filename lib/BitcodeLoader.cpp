#include "mull/BitcodeLoader.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Parallelization.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MD5.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <fstream>
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

std::vector<std::unique_ptr<Bitcode>> BitcodeLoader::loadBitcode(const Configuration &config,
                                                                 Diagnostics &diagnostics) {
  std::vector<std::unique_ptr<Bitcode>> bitcode;

  std::vector<BitcodeLoadingTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(diagnostics, *this);
  }

  TaskExecutor<BitcodeLoadingTask> loader(
      diagnostics, "Loading bitcode", config.bitcodePaths, bitcode, tasks);
  loader.execute();

  return bitcode;
}

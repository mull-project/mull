#include "mull/BitcodeLoader.h"

#include "LLVMCompatibility.h"
#include "mull/Config/Configuration.h"
#include "mull/Logger.h"
#include "mull/Parallelization/Parallelization.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MD5.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <fstream>
#include <iostream>

using namespace llvm;
using namespace mull;

static std::string MD5HashFromBuffer(StringRef buffer) {
  MD5 hasher;
  hasher.update(buffer);
  MD5::MD5Result hash;
  hasher.final(hash);
  SmallString<32> result;
  MD5::stringifyResult(hash, result);
  return result.str();
}

std::pair<std::string, std::unique_ptr<Module>>
mull::loadModuleFromBuffer(LLVMContext &context, MemoryBuffer &buffer) {
  auto module = llvm_compat::parseBitcode(buffer.getMemBufferRef(), context);
  std::string md5 = MD5HashFromBuffer(buffer.getBuffer());
  return std::make_pair(md5, std::move(module));
}

std::unique_ptr<Bitcode>
BitcodeLoader::loadBitcodeAtPath(const std::string &path,
                                 llvm::LLVMContext &context) {
  auto buffer = MemoryBuffer::getFile(path);
  if (!buffer) {
    Logger::error() << "Cannot load module " << path << '\n';
    return nullptr;
  }

  MemoryBuffer *b = buffer.get().get();
  auto modulePair = loadModuleFromBuffer(context, *b);

  std::string hash = modulePair.first;
  std::unique_ptr<llvm::Module> module(std::move(modulePair.second));

  if (module == nullptr) {
    Logger::error() << "Cannot parse bitcode " << path << '\n';
    return nullptr;
  }

  return make_unique<Bitcode>(std::move(module), std::move(buffer.get()), hash);
}

std::vector<std::unique_ptr<Bitcode>>
BitcodeLoader::loadBitcode(const Configuration &config) {
  std::vector<std::unique_ptr<Bitcode>> bitcode;

  std::vector<BitcodeLoadingTask> tasks;
  for (int i = 0; i < config.parallelization.workers; i++) {
    auto context = llvm::make_unique<LLVMContext>();
    tasks.emplace_back(*context, *this);
    contexts.push_back(std::move(context));
  }

  TaskExecutor<BitcodeLoadingTask> loader("Loading bitcode",
                                          config.bitcodePaths, bitcode, tasks);
  loader.execute();

  return bitcode;
}

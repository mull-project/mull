#include "mull/BitcodeLoader.h"

#include "LLVMCompatibility.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Parallelization.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MD5.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <fstream>
#include <sstream>

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

std::pair<std::string, std::unique_ptr<Module>> mull::loadModuleFromBuffer(LLVMContext &context,
                                                                           MemoryBuffer &buffer) {
  auto module = llvm_compat::parseBitcode(buffer.getMemBufferRef(), context);
  std::string md5 = MD5HashFromBuffer(buffer.getBuffer());
  return std::make_pair(md5, std::move(module));
}

std::unique_ptr<Bitcode> BitcodeLoader::loadBitcodeAtPath(const std::string &path,
                                                          llvm::LLVMContext &context,
                                                          Diagnostics &diagnostics) {
  auto buffer = MemoryBuffer::getFile(path);
  if (!buffer) {
    std::stringstream message;
    message << "Cannot load module " << path << '\n';
    diagnostics.error(message.str());
    return nullptr;
  }

  MemoryBuffer *b = buffer.get().get();
  auto modulePair = loadModuleFromBuffer(context, *b);

  std::string hash = modulePair.first;
  std::unique_ptr<llvm::Module> module(std::move(modulePair.second));

  if (module == nullptr) {
    std::stringstream message;
    message << "Cannot parse bitcode " << path << '\n';
    diagnostics.error(message.str());
    return nullptr;
  }

  return make_unique<Bitcode>(std::move(module), std::move(buffer.get()), hash);
}

std::vector<std::unique_ptr<Bitcode>> BitcodeLoader::loadBitcode(const Configuration &config,
                                                                 Diagnostics &diagnostics) {
  std::vector<std::unique_ptr<Bitcode>> bitcode;

  std::vector<BitcodeLoadingTask> tasks;
  for (int i = 0; i < config.parallelization.workers; i++) {
    auto context = llvm::make_unique<LLVMContext>();
    tasks.emplace_back(diagnostics, *context, *this);
    contexts.push_back(std::move(context));
  }

  TaskExecutor<BitcodeLoadingTask> loader(
      diagnostics, "Loading bitcode", config.bitcodePaths, bitcode, tasks);
  loader.execute();

  return bitcode;
}

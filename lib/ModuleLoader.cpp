#include "ModuleLoader.h"

#include "Logger.h"
#include "LLVMCompatibility.h"
#include "Parallelization/Parallelization.h"
#include "Config/Configuration.h"

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
  MD5 Hasher;
  Hasher.update(buffer);
  MD5::MD5Result Hash;
  Hasher.final(Hash);
  SmallString<32> Result;
  MD5::stringifyResult(Hash, Result);
  return Result.str();
}

std::unique_ptr<MullModule>
ModuleLoader::loadModuleAtPath(const std::string &path,
                               llvm::LLVMContext &context) {
  auto buffer = MemoryBuffer::getFile(path);
  if (!buffer) {
    Logger::error() << "Cannot load module " << path << '\n';
    return nullptr;
  }

  std::string hash = MD5HashFromBuffer(buffer->get()->getBuffer());

  auto module = parseBitcodeFile(buffer->get()->getMemBufferRef(), context);
  if (!module) {
    Logger::error() << "Cannot parse bitcode " << path << '\n';
    return nullptr;
  }

  return make_unique<MullModule>(std::move(module.get()), std::move(buffer.get()), hash);
}

std::vector<std::unique_ptr<MullModule>>
ModuleLoader::loadModules(const Configuration &config) {
  std::vector<std::unique_ptr<MullModule>> modules;

  std::vector<ModuleLoadingTask> tasks;
  for (int i = 0; i < config.parallelization.workers; i++) {
    auto context = llvm::make_unique<LLVMContext>();
    tasks.emplace_back(*context, *this);
    contexts.push_back(std::move(context));
  }

  TaskExecutor<ModuleLoadingTask> loader("Loading bitcode", config.bitcodePaths,
                                         modules, tasks);
  loader.execute();

  return modules;
}

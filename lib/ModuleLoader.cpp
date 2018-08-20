#include "ModuleLoader.h"

#include "Logger.h"
#include "LLVMCompatibility.h"
#include "Parallelization/Parallelization.h"
#include "Config.h"

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
  auto BufferOrError = MemoryBuffer::getFile(path);
  if (!BufferOrError) {
    Logger::error() << "ModuleLoader> Can't load module " << path << '\n';
    return nullptr;
  }

  std::string hash = MD5HashFromBuffer(BufferOrError->get()->getBuffer());

  auto llvmModule = parseBitcodeFile(BufferOrError->get()->getMemBufferRef(), context);
  if (!llvmModule) {
    Logger::error() << "ModuleLoader> Can't load module " << path << '\n';
    return nullptr;
  }

  auto module = make_unique<MullModule>(std::move(llvmModule.get()), hash, path);
  return module;
}

std::vector<std::unique_ptr<MullModule>>
ModuleLoader::loadModulesFromBitcodeFileList(const std::vector<std::string> &bitcodeFileList,
                                             Config &config) {
  std::vector<std::unique_ptr<MullModule>> modules;

  std::vector<ModuleLoadingTask> tasks;
  for (int i = 0; i < config.parallelization().workers; i++) {
    auto context = llvm::make_unique<LLVMContext>();
    tasks.emplace_back(*context, *this);
    contexts.push_back(std::move(context));
  }

  TaskExecutor<ModuleLoadingTask> loader("Loading bitcode", bitcodeFileList, modules, tasks);
  loader.execute();

  return modules;
}

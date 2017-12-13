#include "ModuleLoader.h"

#include "Logger.h"

#include <llvm/AsmParser/Parser.h>

// TODO: uncomment when upgrading back to LLVM 4.0
// #include "llvm/Bitcode/BitcodeReader.h"
#include <llvm/Bitcode/ReaderWriter.h>
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

MullModule ModuleLoader::loadModuleAtPath(const std::string &path) {
  auto BufferOrError = MemoryBuffer::getFile(path);
  if (!BufferOrError) {
    Logger::error() << "ModuleLoader> Can't load module " << path << '\n';
    return MullModule::invalidModule();
  }

  std::string hash = MD5HashFromBuffer(BufferOrError->get()->getBuffer());

  auto llvmModule = parseBitcodeFile(BufferOrError->get()->getMemBufferRef(), Ctx);
  if (!llvmModule) {
    Logger::error() << "ModuleLoader> Can't load module " << path << '\n';
    return MullModule::invalidModule();
  }

  return MullModule::validModule(std::move(llvmModule.get()), hash, path);
}

std::vector<MullModule>
ModuleLoader::loadModulesFromBitcodeFileList(const std::vector<std::string> &bitcodeFileList) {
  std::vector<MullModule> modules;

  for (const std::string &path : bitcodeFileList) {
    MullModule module = loadModuleAtPath(path);

    if (module.getModule() == nullptr) {
      continue;
    }

    modules.push_back(std::move(module));
  }

  return modules;
}

#include "MullModule.h"
#include "Logger.h"
#include "LLVMCompatibility.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Path.h>
#include <MullModule.h>

using namespace mull;
using namespace llvm;
using namespace std;

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule)
  : module(std::move(llvmModule)),
    uniqueIdentifier("")
{
}

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule,
                       const std::string &md5,
                       const std::string &path)
: module(std::move(llvmModule)), modulePath(path)
{
  uniqueIdentifier =
    llvm::sys::path::stem(module->getModuleIdentifier()).str() + "_" + md5;
}

std::unique_ptr<MullModule> MullModule::clone(LLVMContext &context) {
  auto bufferOrError = MemoryBuffer::getFile(modulePath);
  if (!bufferOrError) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return nullptr;
  }

  auto llvmModule = parseBitcodeFile(bufferOrError->get()->getMemBufferRef(), context);
  if (!llvmModule) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return nullptr;
  }

  auto module = make_unique<MullModule>(std::move(llvmModule.get()), "", modulePath);
  return module;
}

llvm::Module *MullModule::getModule() {
  return module.get();
}

llvm::Module *MullModule::getModule() const {
  return module.get();
}

std::string MullModule::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MullModule::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

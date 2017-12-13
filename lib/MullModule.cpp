#include "MullModule.h"
#include "Logger.h"

#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

using namespace mull;
using namespace llvm;
using namespace std;

/// FIXME: put into some proper place
static string fileNameFromPath(const string &path) {
  std::string filename;
  auto lastSeparatorPos = path.rfind("/");
  if (lastSeparatorPos == string::npos) {
    filename = path;
  } else {
    filename = path.substr(lastSeparatorPos + 1);
  }

  auto dotPos = filename.find(".");
  if (dotPos != string::npos) {
    filename = filename.substr(0, dotPos);
  }

  return filename;
}

MullModule::MullModule()
: module(unique_ptr<Module>(nullptr)),
  uniqueIdentifier(),
  modulePath()
{
}

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule,
                       const std::string &md5,
                       const std::string &path)
: module(std::move(llvmModule)),
  uniqueIdentifier(fileNameFromPath(module->getModuleIdentifier()) + "_" + md5),
  modulePath(path)
{
}

MullModule MullModule::invalidModule() {
  return MullModule();
}

MullModule MullModule::validModule(std::unique_ptr<llvm::Module> llvmModule,
                                   const std::string &md5,
                                   const std::string &path) {
  return MullModule(std::move(llvmModule), md5, path);
}

MullModule::MullModule(MullModule &&that) {
  this->module = std::move(that.module);
  this->uniqueIdentifier = std::move(that.uniqueIdentifier);
  this->modulePath = std::move(that.modulePath);
}

bool MullModule::isInvalid() const {
  return this->module.get() == nullptr;
}

MullModule MullModule::clone(LLVMContext &context) const {
  auto bufferOrError = MemoryBuffer::getFile(modulePath);
  if (!bufferOrError) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return MullModule::invalidModule();
  }

  auto llvmModule = parseBitcodeFile(bufferOrError->get()->getMemBufferRef(), context);
  if (!llvmModule) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return MullModule::invalidModule();
  }

  return MullModule(std::move(llvmModule.get()), "", modulePath);
}

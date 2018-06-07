#include "MullModule.h"
#include "Logger.h"
#include "LLVMCompatibility.h"

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
  uniqueIdentifier = fileNameFromPath(module->getModuleIdentifier()) + "_" + md5;
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

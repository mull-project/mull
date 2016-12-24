#include "MullModule.h"

#include "llvm/Transforms/Utils/Cloning.h"

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
                           const std::string &md5) : module(std::move(llvmModule))
{
  uniqueIdentifier = fileNameFromPath(module->getModuleIdentifier()) + "_" + md5;
}

std::unique_ptr<MullModule> MullModule::clone() {
  auto llvmModule = CloneModule(module.get());
  auto clone = new MullModule(std::move(llvmModule));
  clone->uniqueIdentifier = uniqueIdentifier;
  return std::unique_ptr<MullModule>(clone);
}

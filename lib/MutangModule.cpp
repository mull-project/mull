#include "MutangModule.h"

#include "llvm/Transforms/Utils/Cloning.h"

using namespace Mutang;
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

MutangModule::MutangModule(std::unique_ptr<llvm::Module> llvmModule)
  : module(std::move(llvmModule)),
    uniqueIdentifier("")
{
}

MutangModule::MutangModule(std::unique_ptr<llvm::Module> llvmModule,
                           const std::string &md5) : module(std::move(llvmModule))
{
  uniqueIdentifier = fileNameFromPath(module->getModuleIdentifier()) + "_" + md5;
}

std::unique_ptr<MutangModule> MutangModule::clone() {
  auto llvmModule = CloneModule(module.get());
  auto clone = new MutangModule(std::move(llvmModule));
  clone->uniqueIdentifier = uniqueIdentifier;
  return std::unique_ptr<MutangModule>(clone);
}

#pragma once

#include "MullModule.h"

#include <string>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
}

namespace mull {

class ModuleLoader {
  std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;
public:
  ModuleLoader() = default;
  virtual ~ModuleLoader() = default;

  virtual std::unique_ptr<MullModule> loadModuleAtPath(const std::string &path,
                                                       llvm::LLVMContext &context);

  virtual std::vector<std::unique_ptr<MullModule>>
    loadModulesFromBitcodeFileList(const std::vector<std::string> &path);
};

}

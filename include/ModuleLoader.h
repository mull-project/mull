#pragma once

#include <string>

#include "MutangModule.h"

namespace llvm {
  class LLVMContext;
  class Module;
}

namespace mull {

class ModuleLoader {
  llvm::LLVMContext &Ctx;
public:
  ModuleLoader(llvm::LLVMContext &C) : Ctx(C) {}
  virtual std::unique_ptr<MutangModule> loadModuleAtPath(const std::string &path);
  virtual ~ModuleLoader() {}
};

}

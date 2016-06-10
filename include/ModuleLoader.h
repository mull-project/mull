#pragma once

#include <string>

namespace llvm {
  class LLVMContext;
  class Module;
}

namespace Mutang {

class ModuleLoader {
  llvm::LLVMContext &Ctx;
public:
  ModuleLoader(llvm::LLVMContext &C) : Ctx(C) {}
  virtual std::unique_ptr<llvm::Module> loadModuleAtPath(const std::string &path);
  virtual ~ModuleLoader() {}
};

}

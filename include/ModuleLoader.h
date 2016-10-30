#pragma once

#include <string>

namespace llvm {
  class LLVMContext;
  class Module;
}

namespace Mutang {

class MutangModule {
  std::unique_ptr<llvm::Module> module;
  std::string moduleMD5;
public:
  MutangModule(std::unique_ptr<llvm::Module> llvmModule, const std::string &md5)
  : module(std::move(llvmModule)), moduleMD5(md5) {}

  llvm::Module *getModule() {
    assert(module.get());
    return module.get();
  }

  llvm::Module *getModule() const {
    assert(module.get());
    return module.get();
  }

  std::string getHash()  {
    return moduleMD5;
  }

  std::string getHash() const {
    return moduleMD5;
  }
};

class ModuleLoader {
  llvm::LLVMContext &Ctx;
public:
  ModuleLoader(llvm::LLVMContext &C) : Ctx(C) {}
  virtual std::unique_ptr<MutangModule> loadModuleAtPath(const std::string &path);
  virtual ~ModuleLoader() {}
};

}

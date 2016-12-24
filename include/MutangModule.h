#pragma once 

#include <string>

#include "llvm/IR/Module.h"

namespace mull {

  class MutangModule {
    std::unique_ptr<llvm::Module> module;
    std::string uniqueIdentifier;
    MutangModule(std::unique_ptr<llvm::Module> llvmModule);
  public:
    MutangModule(std::unique_ptr<llvm::Module> llvmModule,
                 const std::string &md5);

    std::unique_ptr<MutangModule> clone();

    llvm::Module *getModule() {
      assert(module.get());
      return module.get();
    }

    llvm::Module *getModule() const {
      assert(module.get());
      return module.get();
    }

    std::string getUniqueIdentifier() {
      return uniqueIdentifier;
    }

    std::string getUniqueIdentifier() const {
      return uniqueIdentifier;
    }
  };

}

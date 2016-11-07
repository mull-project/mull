#pragma once 

#include <string>

#include "llvm/IR/Module.h"

namespace Mutang {

  class MutangModule {
    std::unique_ptr<llvm::Module> module;
    std::string uniqueIdentifier;
  public:
    MutangModule(std::unique_ptr<llvm::Module> llvmModule,
                 const std::string &md5);

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

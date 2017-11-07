#pragma once 

#include <string>

#include <llvm/IR/Module.h>

namespace llvm {
class LLVMContext;
}

namespace mull {

  class MullModule {
    std::unique_ptr<llvm::Module> module;
    std::string uniqueIdentifier;
    std::string modulePath;
    MullModule(std::unique_ptr<llvm::Module> llvmModule);
  public:
    MullModule(std::unique_ptr<llvm::Module> llvmModule,
               const std::string &md5,
               const std::string &path);

    std::unique_ptr<MullModule> clone(llvm::LLVMContext &context);

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

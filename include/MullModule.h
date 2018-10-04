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
    explicit MullModule(std::unique_ptr<llvm::Module> llvmModule);
  public:
    MullModule(std::unique_ptr<llvm::Module> llvmModule,
               const std::string &md5,
               const std::string &path);

    std::unique_ptr<MullModule> clone(llvm::LLVMContext &context);

    llvm::Module *getModule();
    llvm::Module *getModule() const;
    std::string getUniqueIdentifier();
    std::string getUniqueIdentifier() const;
  };

}

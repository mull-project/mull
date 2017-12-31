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

    MullModule();
    MullModule(std::unique_ptr<llvm::Module> llvmModule,
               const std::string &md5,
               const std::string &path);
  public:
    static MullModule invalidModule();
    static MullModule validModule(std::unique_ptr<llvm::Module> llvmModule,
                                  const std::string &md5,
                                  const std::string &path);

    MullModule(MullModule &&that);

    MullModule(const MullModule &) = delete;
    void operator=(const MullModule &) = delete;
    void operator=(MullModule &&that) = delete;
    static void* operator new(size_t) = delete;
    static void* operator new[](std::size_t) = delete;

    MullModule clone(llvm::LLVMContext &context) const;

    bool isInvalid() const;

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

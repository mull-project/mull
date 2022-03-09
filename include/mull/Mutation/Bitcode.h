#pragma once

#include <string>
#include <thread>

#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>

namespace llvm {
class LLVMContext;
}

namespace mull {

class MutationPoint;
class Diagnostics;

class Bitcode {
public:
  explicit Bitcode(llvm::Module *unownedModule);
  Bitcode(std::unique_ptr<llvm::LLVMContext> context, std::unique_ptr<llvm::Module> module);

  llvm::Module *getModule();
  llvm::Module *getModule() const;
  std::string getUniqueIdentifier();
  std::string getUniqueIdentifier() const;

  void addMutation(MutationPoint *point);

  std::map<llvm::Function *, std::vector<MutationPoint *>> &getMutationPointsMap();

private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  llvm::Module *unownedModule;
  std::string uniqueIdentifier;

  std::map<llvm::Function *, std::vector<MutationPoint *>> mutationPoints;
};

} // namespace mull

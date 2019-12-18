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
class JITEngine;
class Diagnostics;

class Bitcode {
public:
  Bitcode(std::unique_ptr<llvm::Module> module,
          std::unique_ptr<llvm::MemoryBuffer> buffer, const std::string &md5);

  std::unique_ptr<Bitcode> clone(llvm::LLVMContext &context, Diagnostics &diagnostics);

  llvm::Module *getModule();
  llvm::Module *getModule() const;
  std::string getUniqueIdentifier();
  std::string getUniqueIdentifier() const;

  std::string getInstrumentedUniqueIdentifier() const;
  std::string getMutatedUniqueIdentifier() const;

  void addMutation(MutationPoint *point);

  std::map<llvm::Function *, std::vector<MutationPoint *>> &
  getMutationPointsMap();

private:
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::MemoryBuffer> buffer;
  std::string uniqueIdentifier;

  std::map<llvm::Function *, std::vector<MutationPoint *>> mutationPoints;

  explicit Bitcode(std::unique_ptr<llvm::Module> module);
};

} // namespace mull

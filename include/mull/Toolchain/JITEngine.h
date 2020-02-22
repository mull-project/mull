#pragma once

#include "LLVMCompatibility.h"

namespace mull {

class Diagnostics;

class JITEngine {
public:
  explicit JITEngine(Diagnostics &diagnostics);
  void addObjectFiles(std::vector<llvm::object::ObjectFile *> &files,
                      llvm_compat::SymbolResolver &resolver,
                      std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memoryManager);
  llvm_compat::JITSymbol &getSymbol(llvm::StringRef name);

private:
  Diagnostics &diagnostics;
  std::vector<llvm::object::ObjectFile *> objectFiles;
  llvm::StringMap<llvm_compat::JITSymbol> symbolTable;
  llvm_compat::JITSymbol symbolNotFound;
  std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memoryManager;
};

} // namespace mull

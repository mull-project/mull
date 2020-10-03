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
  llvm::JITSymbol &getSymbol(llvm::StringRef name);

  static uint64_t symbolAddress(llvm::JITSymbol &symbol);

private:
  Diagnostics &diagnostics;
  std::vector<llvm::object::ObjectFile *> objectFiles;
  llvm::StringMap<llvm::JITSymbol> symbolTable;
  llvm::JITSymbol symbolNotFound;
  std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memoryManager;
};

} // namespace mull

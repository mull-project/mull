#pragma once

#include <llvm/ExecutionEngine/Orc/JITSymbol.h>

namespace mull {

class JITEngine {
  std::vector<llvm::object::ObjectFile *> objectFiles;
  llvm::StringMap<llvm::orc::JITSymbol> symbolTable;
  llvm::orc::JITSymbol symbolNotFound;
  std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memoryManager;
public:
  JITEngine();
  void addObjectFiles(std::vector<llvm::object::ObjectFile *> &files,
                      llvm::RuntimeDyld::SymbolResolver  &resolver,
                      std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memoryManager);
  llvm::orc::JITSymbol &getSymbol(llvm::StringRef name);
};

}

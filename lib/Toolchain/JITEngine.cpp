#include "mull/Toolchain/JITEngine.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <sstream>
#include <unordered_set>

using namespace mull;
using namespace llvm;

JITEngine::JITEngine(Diagnostics &diagnostics)
    : diagnostics(diagnostics), symbolNotFound(nullptr) {}

void JITEngine::addObjectFiles(std::vector<object::ObjectFile *> &files,
                               llvm_compat::SymbolResolver &resolver,
                               std::unique_ptr<llvm::RuntimeDyld::MemoryManager> memManager) {
  std::vector<object::ObjectFile *>().swap(objectFiles);
  llvm::StringMap<llvm_compat::JITSymbolInfo>().swap(symbolTable);
  memoryManager = std::move(memManager);

  std::unordered_set<std::string> unresolvedSymbols;

  for (auto object : files) {
    objectFiles.push_back(object);

    for (auto symbol : object->symbols()) {
      Expected<StringRef> nameOrError = symbol.getName();
      if (!nameOrError) {
        consumeError(nameOrError.takeError());
        continue;
      }
      std::string name = nameOrError.get();
      if (symbol.getFlags() & object::SymbolRef::SF_Undefined) {
        if (!(symbol.getFlags() & object::SymbolRef::SF_Weak)) {
          unresolvedSymbols.insert(name);
        }
        continue;
      }
      unresolvedSymbols.erase(name);
      auto flags = llvm_compat::JITSymbolFlagsFromObjectSymbol(symbol);
      symbolTable.insert(std::make_pair(name, llvm_compat::JITSymbol(0, flags)));
    }
  }

  RuntimeDyld dynamicLoader(*memoryManager, resolver);
  dynamicLoader.setProcessAllSections(false);

  for (auto &object : objectFiles) {
    dynamicLoader.loadObject(*object);
  }

  for (auto &entry : symbolTable) {
    entry.second = dynamicLoader.getSymbol(entry.first());
  }

  dynamicLoader.finalizeWithMemoryManagerLocking();

  auto it = unresolvedSymbols.begin();
  while (it != unresolvedSymbols.end()) {
    std::string name = *it;
    auto dlSymbol = dynamicLoader.getSymbol(name);
    if (dlSymbol.getAddress() != 0) {
      it = unresolvedSymbols.erase(it);
      continue;
    }
    auto resolverSymbol = llvm_compat::JITSymbol(resolver.findSymbol(name));
    if (llvm_compat::JITSymbolAddress(resolverSymbol) != 0) {
      it = unresolvedSymbols.erase(it);
      continue;
    }
    ++it;
  }

  if (!unresolvedSymbols.empty()) {
    std::stringstream stringstream;
    stringstream << "JIT engine could not resolve the following symbols:\n";
    for (const std::string &name : unresolvedSymbols) {
      stringstream << name << "\n";
    }
    diagnostics.warning(stringstream.str());
  }
}

llvm_compat::JITSymbol &JITEngine::getSymbol(llvm::StringRef name) {
  auto symbolIterator = symbolTable.find(name);
  if (symbolIterator == symbolTable.end()) {
    return symbolNotFound;
  }

  return symbolIterator->second;
}

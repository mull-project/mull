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
  llvm::StringMap<llvm::JITSymbol>().swap(symbolTable);
  memoryManager = std::move(memManager);

  std::unordered_set<std::string> unresolvedSymbols;

  diagnostics.debug("Building symbol table");
  for (auto object : files) {
    objectFiles.push_back(object);

    for (auto symbol : object->symbols()) {
      Expected<StringRef> nameOrError = symbol.getName();
      if (!nameOrError) {
        consumeError(nameOrError.takeError());
        continue;
      }
      std::string name = nameOrError.get().str();
      if (llvm_compat::flagsFromSymbol(symbol) & object::SymbolRef::SF_Undefined) {
        if (!(llvm_compat::flagsFromSymbol(symbol) & object::SymbolRef::SF_Weak)) {
          unresolvedSymbols.insert(name);
        }
        continue;
      }
      unresolvedSymbols.erase(name);
      auto flags = llvm_compat::JITSymbolFlagsFromObjectSymbol(symbol);
      symbolTable.insert(std::make_pair(name, llvm::JITSymbol(0, flags)));
    }
  }

  RuntimeDyld dynamicLoader(*memoryManager, resolver);
  dynamicLoader.setProcessAllSections(false);

  for (auto &object : objectFiles) {
    std::ostringstream ss;
    ss << "Load object " << object->getFileName().str();
    diagnostics.debug(ss.str());
    dynamicLoader.loadObject(*object);
  }

  for (auto &entry : symbolTable) {
    entry.second = dynamicLoader.getSymbol(entry.first());
  }

  dynamicLoader.finalizeWithMemoryManagerLocking();

  diagnostics.debug("Resolving symbol references");
  auto it = unresolvedSymbols.begin();
  while (it != unresolvedSymbols.end()) {
    const std::string &name = *it;
    auto dlSymbol = dynamicLoader.getSymbol(name);
    if (dlSymbol.getAddress() != 0) {
      std::ostringstream ss;
      ss << "Found static symbol " << name;
      diagnostics.debug(ss.str());
      it = unresolvedSymbols.erase(it);
      continue;
    }
    auto resolverSymbol = llvm::JITSymbol(resolver.findSymbol(name));
    if (JITEngine::symbolAddress(resolverSymbol) != 0) {
      std::ostringstream ss;
      ss << "Found dynamic library symbol " << name;
      diagnostics.debug(ss.str());
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

llvm::JITSymbol &JITEngine::getSymbol(llvm::StringRef name) {
  auto symbolIterator = symbolTable.find(name);
  if (symbolIterator == symbolTable.end()) {
    return symbolNotFound;
  }

  return symbolIterator->second;
}

uint64_t JITEngine::symbolAddress(llvm::JITSymbol &symbol) {
  auto addressOrError = symbol.getAddress();
  if (!addressOrError) {
    consumeError(addressOrError.takeError());
    return 0;
  }

  return addressOrError.get();
}

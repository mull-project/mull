#include "mull/Program/Program.h"

#include <llvm/IR/Constants.h>
#include <utility>

using namespace mull;

Program::Program(std::vector<std::unique_ptr<Bitcode>> bitcode) {
  for (auto &bc : bitcode) {
    addBitcode(std::move(bc));
  }
}

std::vector<std::unique_ptr<Bitcode>> &Program::bitcode() {
  return _bitcode;
}

void Program::addBitcode(std::unique_ptr<Bitcode> bitcode) {
  for (auto &function : bitcode->getModule()->getFunctionList()) {
    if (!function.isDeclaration()) {
      functionsRegistry.insert(std::make_pair(function.getName(), &function));
    }
  }

  for (auto &alias : bitcode->getModule()->getAliasList()) {
    if (auto function = llvm::dyn_cast<llvm::Function>(alias.getAliasee())) {
      functionsRegistry.insert(std::make_pair(alias.getName(), function));
    }
  }

  std::string identifier = bitcode->getModule()->getModuleIdentifier();

  assert(bitcodeWithIdentifier(identifier) == nullptr &&
         "Attempt to add a module which has been added already!");

  bitcodeRegistry.insert(std::make_pair(identifier, bitcode.get()));
  _bitcode.emplace_back(std::move(bitcode));
}

Bitcode *Program::bitcodeWithIdentifier(const std::string &identifier) const {
  auto it = bitcodeRegistry.find(identifier);
  if (it == bitcodeRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

llvm::Function *Program::lookupDefinedFunction(llvm::StringRef FunctionName) const {
  auto it = functionsRegistry.find(FunctionName.str());
  if (it == functionsRegistry.end()) {
    return nullptr;
  }
  return it->second;
}

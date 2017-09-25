#pragma once

#include "ModuleLoader.h"

#include <llvm/IR/Module.h>

#include <map>
#include <string>
#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class Context {
public:
  typedef std::vector<std::unique_ptr<llvm::Module>> ModuleArrayType;
  typedef ModuleArrayType::iterator iterator;

private:
  ModuleArrayType Modules;
  std::map<std::string, llvm::Function *> FunctionsRegistry;
  std::map<std::string, llvm::Module *> moduleRegistry;

public:
  void addModule(std::unique_ptr<llvm::Module> module);

  std::vector<llvm::Function *> getStaticConstructors();

  llvm::Module *moduleWithIdentifier(const std::string &identifier);
  llvm::Module *moduleWithIdentifier(const std::string &identifier) const;

  ModuleArrayType &getModules() { return Modules; }
  llvm::Function *lookupDefinedFunction(llvm::StringRef FunctionName) const;
  iterator begin()  { return Modules.begin(); }
  iterator end()    { return Modules.end();   }
};

}

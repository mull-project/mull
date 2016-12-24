#pragma once

#include "ModuleLoader.h"

#include "llvm/IR/Module.h"

#include <map>
#include <string>
#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class Context {
public:
  typedef std::vector<std::unique_ptr<MutangModule>> ModuleArrayType;
  typedef ModuleArrayType::iterator iterator;

private:
  ModuleArrayType Modules;
  std::map<std::string, llvm::Function *> FunctionsRegistry;
  std::map<std::string, MutangModule *> moduleRegistry;

public:
  void addModule(std::unique_ptr<MutangModule> module);

  std::vector<llvm::Function *> getStaticConstructors();

  MutangModule *moduleWithIdentifier(const std::string &identifier);
  MutangModule *moduleWithIdentifier(const std::string &identifier) const;

  ModuleArrayType &getModules() { return Modules; }
  llvm::Function *lookupDefinedFunction(llvm::StringRef FunctionName);
  iterator begin()  { return Modules.begin(); }
  iterator end()    { return Modules.end();   }
};

}

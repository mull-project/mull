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
  typedef std::vector<MullModule> ModuleArrayType;
  typedef ModuleArrayType::iterator iterator;

private:
  ModuleArrayType Modules;
  std::map<std::string, llvm::Function *> FunctionsRegistry;
  std::map<std::string, MullModule *> moduleRegistry;

  const MullModule invalidModule;
public:
  Context() : invalidModule(MullModule::invalidModule()) {}
  void addModule(MullModule module);
  void reserveSpace(size_t size);

  std::vector<llvm::Function *> getStaticConstructors();

  const MullModule &moduleWithIdentifier(const std::string &identifier);
  const MullModule &moduleWithIdentifier(const std::string &identifier) const;

  ModuleArrayType &getModules() { return Modules; }
  llvm::Function *lookupDefinedFunction(llvm::StringRef FunctionName);
  iterator begin()  { return Modules.begin(); }
  iterator end()    { return Modules.end();   }
};

}

#pragma once

#include <map>
#include <vector>

//#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Module.h"

namespace llvm {

class Function;

}

namespace Mutang {

class Context {
public:
  typedef std::vector<std::unique_ptr<llvm::Module>> ModuleArrayType;
  typedef ModuleArrayType::iterator iterator;

private:
  ModuleArrayType Modules;
//  llvm::StringMap<llvm::Function *> FunctionsRegistry;
  std::map<std::string, llvm::Function *> FunctionsRegistry;

public:
  void addModule(std::unique_ptr<llvm::Module> M);

  std::vector<llvm::Function *> getStaticConstructors();

  ModuleArrayType &getModules() { return Modules; }
  llvm::Function *lookupDefinedFunction(llvm::StringRef FunctionName);
  iterator begin()  { return Modules.begin(); }
  iterator end()    { return Modules.end();   }
};

}

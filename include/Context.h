#pragma once

#include <vector>

#include "llvm/IR/Module.h"

namespace Mutang {

class Context {
public:
  typedef std::vector<llvm::Module *> ModuleArrayType;
  typedef ModuleArrayType::iterator iterator;

private:
  ModuleArrayType Modules;

public:
  void addModule(std::unique_ptr<llvm::Module> M);

  ModuleArrayType getModules() { return Modules; }
  iterator begin()  { return Modules.begin(); }
  iterator end()    { return Modules.end();   }
};

}

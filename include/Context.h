#pragma once

#include <vector>

namespace llvm {

class Module;

}

namespace Mutang {

class Context {
public:
  typedef std::vector<llvm::Module *>  ModuleArrayType;
  typedef ModuleArrayType::iterator    iterator;

private:
  ModuleArrayType Modules;

public:
  void addModule(llvm::Module *M);

  ModuleArrayType getModules() { return Modules; }
  iterator begin()  { return Modules.begin(); }
  iterator end()    { return Modules.end();   }
};

}

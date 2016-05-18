#pragma once

#include "llvm/Object/ObjectFile.h"

#include <map>

namespace llvm {
  class Module;
}

namespace Mutang {

class Config;
class ModuleLoader;

class Driver {
  Config &Cfg;
  ModuleLoader &Loader;
  std::map<llvm::Module *, llvm::object::OwningBinary<llvm::object::ObjectFile>> InnerCache;
public:
  Driver(Config &C, ModuleLoader &ML) : Cfg(C), Loader(ML) {}
  void Run();

private:
  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);
};

}

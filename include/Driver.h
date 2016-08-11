#pragma once

#include "TestResult.h"
#include "Context.h"
#include "llvm/Object/ObjectFile.h"

#include <map>

namespace llvm {
  class Module;
  class Function;
}

namespace Mutang {

class Config;
class ModuleLoader;

class Driver {
  Config &Cfg;
  Context Ctx;
  ModuleLoader &Loader;
  std::map<llvm::Module *, llvm::object::OwningBinary<llvm::object::ObjectFile>> InnerCache;
public:
  Driver(Config &C, ModuleLoader &ML) : Cfg(C), Loader(ML) {}
  std::vector<std::unique_ptr<TestResult>> Run();
  std::vector<std::unique_ptr<TestResult>> RunGTest();

private:
  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  /// Returns cached object files for all modules
  std::vector<llvm::object::ObjectFile *> AllObjectFiles();

  std::vector<llvm::Function *> getStaticCtors();
};

}

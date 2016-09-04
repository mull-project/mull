#pragma once

#include "TestResult.h"
#include "ForkProcessSandbox.h"
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
class TestFinder;
class TestRunner;

class Driver {
  Config &Cfg;
  ModuleLoader &Loader;
  TestFinder &Finder;
  TestRunner &Runner;
  Context Ctx;
  ForkProcessSandbox Sandbox;

  std::map<llvm::Module *, llvm::object::OwningBinary<llvm::object::ObjectFile>> InnerCache;
public:
  Driver(Config &C, ModuleLoader &ML, TestFinder &TF, TestRunner &TR)
    : Cfg(C), Loader(ML), Finder(TF), Runner(TR) {}
  std::vector<std::unique_ptr<TestResult>> Run();

private:
  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  /// Returns cached object files for all modules
  std::vector<llvm::object::ObjectFile *> AllObjectFiles();
};

}

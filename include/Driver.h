#pragma once

#include "Config.h"
#include "TestResult.h"
#include "ForkProcessSandbox.h"
#include "Context.h"
#include "MutationOperators/MutationOperator.h"

#include "Toolchain/Toolchain.h"

#include "llvm/Object/ObjectFile.h"
#include "MullJIT.h"

#include <map>

namespace llvm {

class Module;
class Function;

}

namespace mull {

class Config;
class ModuleLoader;
class Result;
class TestFinder;
class TestRunner;

class Driver {
  Config &Cfg;
  ModuleLoader &Loader;
  TestFinder &Finder;
  TestRunner &Runner;
  Toolchain &toolchain;
  Context Ctx;
  ProcessSandbox *Sandbox;
  MullJIT &jit;

  std::map<llvm::Module *, llvm::object::ObjectFile *> InnerCache;

public:
  Driver(Config &C, ModuleLoader &ML, TestFinder &TF, TestRunner &TR, Toolchain &t, MullJIT &jit)
    : Cfg(C), Loader(ML), Finder(TF), Runner(TR), toolchain(t), jit(jit) {
      if (C.getFork()) {
        this->Sandbox = new ForkProcessSandbox();
      } else {
        this->Sandbox = new NullProcessSandbox();
      }
    }

  ~Driver() {
    delete this->Sandbox;
  }

  std::unique_ptr<Result> Run();
  
  static std::vector<std::unique_ptr<MutationOperator>>
    mutationOperators(std::vector<std::string> mutationOperatorStrings);

  static std::vector<std::unique_ptr<MutationOperator>>
    defaultMutationOperators();

private:
  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  /// Returns cached object files for all modules
  // std::vector<llvm::object::ObjectFile *> AllObjectFiles();
  std::vector<llvm::Module *> allModules();
};

}

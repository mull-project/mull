#pragma once

#include "Config.h"
#include "TestResult.h"
#include "ForkProcessSandbox.h"
#include "IDEDiagnostics.h"
#include "Context.h"

#include "Toolchain/Toolchain.h"

#include "llvm/Object/ObjectFile.h"

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
  IDEDiagnostics *diagnostics;

  std::map<llvm::Module *, llvm::object::ObjectFile *> InnerCache;

public:
  Driver(Config &C, ModuleLoader &ML, TestFinder &TF, TestRunner &TR, Toolchain &t)
    : Cfg(C), Loader(ML), Finder(TF), Runner(TR), toolchain(t) {
      if (C.getFork()) {
        this->Sandbox = new ForkProcessSandbox();
      } else {
        this->Sandbox = new NullProcessSandbox();
      }

      if (C.isDiagnosticsEnabled()) {
        this->diagnostics = new NormalIDEDiagnostics();
      } else {
        this->diagnostics = new NullIDEDiagnostics();
      }
    }

  ~Driver() {
    delete this->Sandbox;
    delete this->diagnostics;
  }

  std::unique_ptr<Result> Run();

private:
  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  /// Returns cached object files for all modules
  std::vector<llvm::object::ObjectFile *> AllObjectFiles();
};

}

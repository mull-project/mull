#pragma once

#include "Config.h"
#include "ExecutionResult.h"
#include "MutationResult.h"
#include "ForkProcessSandbox.h"
#include "IDEDiagnostics.h"
#include "Context.h"
#include "MutationOperators/MutationOperator.h"
#include "Instrumentation/DynamicCallTree.h"
#include "Instrumentation/Callbacks.h"

#include "Toolchain/Toolchain.h"

#include <llvm/Object/ObjectFile.h>

#include <map>

namespace llvm {

class Module;
class Function;

}

namespace mull {

class Config;
class Filter;
class ModuleLoader;
class Result;
class TestFinder;
class TestRunner;
class MutationsFinder;

class Driver;

class Driver {
  Config &Cfg;
  ModuleLoader &Loader;
  TestFinder &Finder;
  TestRunner &Runner;
  Toolchain &toolchain;
  Filter &filter;
  MutationsFinder &mutationsFinder;
  Context Ctx;
  ProcessSandbox *Sandbox;
  IDEDiagnostics *diagnostics;
  std::vector<CallTreeFunction> functions;
  DynamicCallTree dynamicCallTree;
  uint64_t *_callTreeMapping;
  std::stack<uint64_t> _callstack;

  std::map<llvm::Module *, llvm::object::ObjectFile *> InnerCache;
  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> precompiledObjectFiles;
  Callbacks callbacks;
public:
  Driver(Config &C, ModuleLoader &ML, TestFinder &TF, TestRunner &TR, Toolchain &t, Filter &f, MutationsFinder &mutationsFinder)
    : Cfg(C), Loader(ML), Finder(TF), Runner(TR), toolchain(t), filter(f), mutationsFinder(mutationsFinder), dynamicCallTree(functions), _callTreeMapping(nullptr), precompiledObjectFiles(), callbacks(t) {

      CallTreeFunction phonyRoot(nullptr);
      functions.push_back(phonyRoot);

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

  ~Driver();

  std::unique_ptr<Result> Run();
  uint64_t *callTreeMapping() {
    return _callTreeMapping;
  }
  std::stack<uint64_t> &callstack() {
    return _callstack;
  }

private:
  void prepareForExecution();

  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  /// Returns cached object files for all modules
  std::vector<llvm::object::ObjectFile *> AllObjectFiles();

  std::vector<std::unique_ptr<MutationResult>> dryRunMutations(const std::vector<MutationPoint *> &mutationPoints);
  std::vector<std::unique_ptr<MutationResult>> runMutations(const std::vector<MutationPoint *> &mutationPoints);
};

}

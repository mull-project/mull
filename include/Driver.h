#pragma once

#include "Config.h"
#include "ExecutionResult.h"
#include "MutationResult.h"
#include "ForkProcessSandbox.h"
#include "IDEDiagnostics.h"
#include "Context.h"
#include "MutationOperators/MutationOperator.h"
#include "Instrumentation/Instrumentation.h"

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
class Metrics;

class Driver {
  Config &config;
  ModuleLoader &loader;
  TestFinder &finder;
  TestRunner &runner;
  Toolchain &toolchain;
  Filter &filter;
  MutationsFinder &mutationsFinder;
  Context context;
  ProcessSandbox *sandbox;
  IDEDiagnostics *diagnostics;

  std::map<llvm::Module *, llvm::object::ObjectFile *> innerCache;
  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> precompiledObjectFiles;
  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> instrumentedObjectFiles;
  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> ownedObjectFiles;
  Instrumentation instrumentation;
  Metrics &metrics;
public:
  Driver(Config &C, ModuleLoader &ML, TestFinder &TF, TestRunner &TR, Toolchain &t, Filter &f, MutationsFinder &mutationsFinder, Metrics &metrics)
    : config(C), loader(ML), finder(TF), runner(TR), toolchain(t), filter(f), mutationsFinder(mutationsFinder), precompiledObjectFiles(), instrumentation(), metrics(metrics) {

      if (C.forkEnabled()) {
        this->sandbox = new ForkProcessSandbox();
      } else {
        this->sandbox = new NullProcessSandbox();
      }

      Config::Diagnostics diagnostics = C.getDiagnostics();
      if (diagnostics != Config::Diagnostics::None) {
        this->diagnostics = new NormalIDEDiagnostics(diagnostics);
      } else {
        this->diagnostics = new NullIDEDiagnostics();
      }
    }

  ~Driver();

  std::unique_ptr<Result> Run();

private:
  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  std::vector<llvm::object::ObjectFile *> AllInstrumentedObjectFiles();

  std::vector<std::unique_ptr<MutationResult>> dryRunMutations(const std::vector<MutationPoint *> &mutationPoints);
  std::vector<std::unique_ptr<MutationResult>> runMutations(const std::vector<MutationPoint *> &mutationPoints);
};

}

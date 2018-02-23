#pragma once

#include "Config.h"
#include "ExecutionResult.h"
#include "MutationResult.h"
#include "ForkProcessSandbox.h"
#include "IDEDiagnostics.h"
#include "Context.h"
#include "MutationOperators/MutationOperator.h"
#include "Instrumentation/Instrumentation.h"
#include "Test.h"
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
class JunkDetector;

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
  JunkDetector &junkDetector;
public:
  Driver(Config &C, ModuleLoader &ML, TestFinder &TF, TestRunner &TR, Toolchain &t, Filter &f, MutationsFinder &mutationsFinder, Metrics &metrics, JunkDetector &junkDetector)
    : config(C), loader(ML), finder(TF), runner(TR), toolchain(t), filter(f), mutationsFinder(mutationsFinder), precompiledObjectFiles(), instrumentation(), metrics(metrics), junkDetector(junkDetector) {

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
  void loadBitcodeFilesIntoMemory();
  void compileInstrumentedBitcodeFiles();
  void loadPrecompiledObjectFiles();
  void loadDynamicLibraries();

  std::vector<std::unique_ptr<Test>> findTests();
  std::vector<MutationPoint *> findMutationPoints(std::vector<std::unique_ptr<Test>> &tests);
  std::vector<MutationPoint *> filterOutJunkMutations(std::vector<MutationPoint *> mutationPoints);

  std::vector<std::unique_ptr<MutationResult>> runMutations(std::vector<MutationPoint *> &mutationPoints);

  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);

  std::vector<llvm::object::ObjectFile *> AllInstrumentedObjectFiles();

  std::vector<std::unique_ptr<MutationResult>> dryRunMutations(const std::vector<MutationPoint *> &mutationPoints);
  std::vector<std::unique_ptr<MutationResult>> normalRunMutations(const std::vector<MutationPoint *> &mutationPoints);
};

}

#pragma once

#include "ExecutionResult.h"
#include "MutationResult.h"
#include "ForkProcessSandbox.h"
#include "IDEDiagnostics.h"
#include "Context.h"
#include "Mutators/Mutator.h"
#include "Instrumentation/Instrumentation.h"
#include "TestFrameworks/Test.h"
#include "Toolchain/Toolchain.h"

#include <llvm/Object/ObjectFile.h>

#include <map>

namespace llvm {

class Module;
class Function;

}

namespace mull {

struct Configuration;

class Filter;
class ModuleLoader;
class Result;
class TestFramework;
class MutationsFinder;
class Metrics;
class JunkDetector;

class Driver {
  const Configuration &config;
  ModuleLoader &loader;
  TestFramework &testFramework;
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
  Driver(const Configuration &config,
         ModuleLoader &ML,
         TestFramework &testFramework,
         Toolchain &t,
         Filter &f,
         MutationsFinder &mutationsFinder,
         Metrics &metrics,
         JunkDetector &junkDetector);

  ~Driver();

  std::unique_ptr<Result> Run();

  /// Returns cached object files for all modules excerpt one provided
  std::vector<llvm::object::ObjectFile *> AllButOne(llvm::Module *One);
private:
  void loadBitcodeFilesIntoMemory();
  void compileInstrumentedBitcodeFiles();
  void loadPrecompiledObjectFiles();
  void loadDynamicLibraries();

  std::vector<std::unique_ptr<Test>> findTests();
  std::vector<MutationPoint *> findMutationPoints(std::vector<std::unique_ptr<Test>> &tests);
  std::vector<MutationPoint *> filterOutJunkMutations(std::vector<MutationPoint *> mutationPoints);

  std::vector<std::unique_ptr<MutationResult>> runMutations(std::vector<MutationPoint *> &mutationPoints);

  std::vector<llvm::object::ObjectFile *> AllInstrumentedObjectFiles();

  std::vector<std::unique_ptr<MutationResult>> dryRunMutations(const std::vector<MutationPoint *> &mutationPoints);
  std::vector<std::unique_ptr<MutationResult>> normalRunMutations(const std::vector<MutationPoint *> &mutationPoints);
};

}

#pragma once

#include "ExecutionResult.h"
#include "MutationResult.h"
#include "ForkProcessSandbox.h"
#include "IDEDiagnostics.h"
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

class Program;
class Filter;
class Result;
class TestFramework;
class MutationsFinder;
class Metrics;
class JunkDetector;

class Driver {
  const Configuration &config;
  Program &program;
  TestFramework &testFramework;
  Toolchain &toolchain;
  Filter &filter;
  MutationsFinder &mutationsFinder;
  ProcessSandbox *sandbox;
  IDEDiagnostics *diagnostics;

  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> instrumentedObjectFiles;
  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> ownedObjectFiles;
  Instrumentation instrumentation;
  Metrics &metrics;
  JunkDetector &junkDetector;
public:
  Driver(const Configuration &config,
         Program &program,
         TestFramework &testFramework,
         Toolchain &t,
         Filter &f,
         MutationsFinder &mutationsFinder,
         Metrics &metrics,
         JunkDetector &junkDetector);

  ~Driver();

  std::unique_ptr<Result> Run();

private:
  void compileInstrumentedBitcodeFiles();
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

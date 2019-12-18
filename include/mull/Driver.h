#pragma once

#include "mull/ExecutionResult.h"
#include "mull/Filters/MutationFilter.h"
#include "mull/IDEDiagnostics.h"
#include "mull/Instrumentation/Instrumentation.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Sandbox/ProcessSandbox.h"
#include "mull/TestFrameworks/Test.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/Object/ObjectFile.h>

#include <map>

namespace llvm {

class Module;
class Function;

} // namespace llvm

namespace mull {

struct Configuration;

class Program;
class Filter;
class Result;
class TestFramework;
class MutationsFinder;
class Metrics;
class JunkDetector;
class FunctionFilter;
struct Filters;
class Diagnostics;

class Driver {
  const Configuration &config;
  Program &program;
  TestFramework &testFramework;
  Toolchain &toolchain;
  MutationsFinder &mutationsFinder;
  const ProcessSandbox &sandbox;
  IDEDiagnostics *ideDiagnostics;
  Diagnostics &diagnostics;

  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>
      instrumentedObjectFiles;
  std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>>
      ownedObjectFiles;
  Instrumentation instrumentation;
  Metrics &metrics;

  struct Filters &filters;

public:
  Driver(Diagnostics &diagnostics, const Configuration &config, const ProcessSandbox &sandbox,
         Program &program, Toolchain &t, Filters &filters, MutationsFinder &mutationsFinder,
         Metrics &metrics, TestFramework &testFramework);

  ~Driver();

  std::unique_ptr<Result> Run();

private:
  void compileInstrumentedBitcodeFiles();
  void loadDynamicLibraries();

  std::vector<Test> findTests();
  std::vector<MutationPoint *> findMutationPoints(std::vector<Test> &tests);
  std::vector<MutationPoint *>
  filterMutations(std::vector<MutationPoint *> mutationPoints);
  std::vector<FunctionUnderTest>
  filterFunctions(std::vector<FunctionUnderTest> functions);
  void selectInstructions(std::vector<FunctionUnderTest> &functions);

  std::vector<std::unique_ptr<MutationResult>>
  runMutations(std::vector<MutationPoint *> &mutationPoints);

  std::vector<llvm::object::ObjectFile *> AllInstrumentedObjectFiles();

  std::vector<std::unique_ptr<MutationResult>>
  dryRunMutations(const std::vector<MutationPoint *> &mutationPoints);
  std::vector<std::unique_ptr<MutationResult>>
  normalRunMutations(const std::vector<MutationPoint *> &mutationPoints);
};

} // namespace mull

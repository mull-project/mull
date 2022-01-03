#pragma once

#include "mull/ExecutionResult.h"
#include "mull/Filters/MutationFilter.h"
#include "mull/IDEDiagnostics.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Toolchain/Toolchain.h"

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
class MutationsFinder;
class JunkDetector;
class FunctionFilter;
struct Filters;
class Diagnostics;

class Driver {
  const Configuration &config;
  Program &program;
  Toolchain &toolchain;
  MutationsFinder &mutationsFinder;
  IDEDiagnostics *ideDiagnostics;
  Diagnostics &diagnostics;

  struct Filters &filters;
  SingleTaskExecutor singleTask;

public:
  Driver(Diagnostics &diagnostics, const Configuration &config, Program &program, Toolchain &t,
         Filters &filters, MutationsFinder &mutationsFinder);

  ~Driver();

  std::unique_ptr<Result> run();

private:
  std::vector<MutationPoint *> findMutationPoints();
  std::vector<MutationPoint *> filterMutations(std::vector<MutationPoint *> mutationPoints);
  std::vector<FunctionUnderTest> filterFunctions(std::vector<FunctionUnderTest> functions);
  void selectInstructions(std::vector<FunctionUnderTest> &functions);

  void prepareMutations(std::vector<MutationPoint *> mutationPoints);

  std::vector<std::unique_ptr<MutationResult>>
  runMutations(std::vector<std::unique_ptr<Mutant>> &mutants);

  std::vector<std::unique_ptr<MutationResult>>
  dryRunMutations(std::vector<std::unique_ptr<Mutant>> &mutants);
  std::vector<std::unique_ptr<MutationResult>>
  normalRunMutations(std::vector<std::unique_ptr<Mutant>> &mutants);

  std::vector<FunctionUnderTest> getFunctionsUnderTest();
};

void mutateBitcode(llvm::Module &module);

} // namespace mull

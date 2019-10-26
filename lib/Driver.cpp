#include "mull/Driver.h"

#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/JunkDetection/JunkDetector.h"
#include "mull/Logger.h"
#include "mull/Metrics/Metrics.h"
#include "mull/MutationResult.h"
#include "mull/MutationsFinder.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"
#include "mull/ReachableFunction.h"
#include "mull/Result.h"
#include "mull/TestFrameworks/TestFramework.h"
#include "mull/Toolchain/JITEngine.h"

#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <sys/mman.h>
#include <sys/types.h>
#include <vector>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;

Driver::~Driver() { delete this->diagnostics; }

/// TODO: Remove the following comments as they are irrelevant
/// Populate mull::Context with modules using
/// ModulePaths from mull::Config.
/// mull::Context should be populated using ModuleLoader
/// so that we could inject modules from string for testing purposes

/// Having mull::Context in place we could instantiate TestFinder and find all
/// tests Using same TestFinder we could find mutation points, apply them
/// sequentially and run tests/mutants using newly created TestRunner

/// This method should return (somehow) results of the tests/mutants execution
/// So that we could easily plug in some TestReporter

/// UPD: The method returns set of results
/// Number of results equals to a number of tests
/// Each result contains result of execution of an original test and
/// all the results of each mutant within corresponding MutationPoint

std::unique_ptr<Result> Driver::Run() {
  compileInstrumentedBitcodeFiles();
  loadDynamicLibraries();

  auto tests = findTests();
  auto mutationPoints = findMutationPoints(tests);
  auto filteredMutations = filterMutations(std::move(mutationPoints));
  auto mutationResults = runMutations(filteredMutations);

  return make_unique<Result>(std::move(tests), std::move(mutationResults),
                             std::move(filteredMutations));
}

void Driver::compileInstrumentedBitcodeFiles() {
  metrics.beginInstrumentedCompilation();

  for (auto &bitcode : program.bitcode()) {
    instrumentation.recordFunctions(bitcode->getModule());
  }

  std::vector<InstrumentedCompilationTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(instrumentation, toolchain);
  }

  TaskExecutor<InstrumentedCompilationTask> compiler(
      "Compiling instrumented code", program.bitcode(), instrumentedObjectFiles,
      tasks);
  compiler.execute();

  metrics.endInstrumentedCompilation();
}

void Driver::loadDynamicLibraries() {
  SingleTaskExecutor task("Loading dynamic libraries", [&]() {
    for (const std::string &dylibPath : program.getDynamicLibraryPaths()) {
      std::string msg;
      auto error =
          sys::DynamicLibrary::LoadLibraryPermanently(dylibPath.c_str(), &msg);
      if (error) {
        Logger::error() << "Cannot load dynamic library '" << dylibPath
                        << "': " << msg << "\n";
      }
    }
  });
  task.execute();
}

std::vector<Test> Driver::findTests() {
  std::vector<Test> tests;
  SingleTaskExecutor task("Searching tests", [&]() {
    tests = testFramework.finder().findTests(program, filter);
  });
  task.execute();
  return tests;
}

std::vector<MutationPoint *> Driver::findMutationPoints(vector<Test> &tests) {
  if (tests.empty()) {
    return std::vector<MutationPoint *>();
  }

  auto objectFiles = AllInstrumentedObjectFiles();
  JITEngine jit;

  SingleTaskExecutor prepareOriginalTestRunTask(
      "Preparing original test run", [&]() {
        testFramework.runner().loadInstrumentedProgram(objectFiles,
                                                       instrumentation, jit);
      });
  prepareOriginalTestRunTask.execute();

  std::vector<OriginalTestExecutionTask> tasks;
  tasks.reserve(config.parallelization.testExecutionWorkers);
  for (int i = 0; i < config.parallelization.testExecutionWorkers; i++) {
    tasks.emplace_back(instrumentation, program, sandbox,
                       testFramework.runner(), config, filter, jit);
  }

  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  TaskExecutor<OriginalTestExecutionTask> testRunner(
      "Running original tests", tests, reachableFunctions, tasks);
  testRunner.execute();

  auto functionsUnderTest = mergeReachableFunctions(reachableFunctions);
  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(program, functionsUnderTest, filter);

  {
    /// Cleans up the memory allocated for the vector itself as well
    std::vector<OwningBinary<ObjectFile>>().swap(instrumentedObjectFiles);
  }

  return mutationPoints;
}

std::vector<MutationPoint *>
Driver::filterMutations(std::vector<MutationPoint *> mutationPoints) {
  std::vector<MutationPoint *> mutations = std::move(mutationPoints);

  for (auto filter : mutationFilters) {
    std::vector<MutationFilterTask> tasks;
    tasks.reserve(config.parallelization.workers);
    for (int i = 0; i < config.parallelization.workers; i++) {
      tasks.emplace_back(*filter);
    }

    std::string label = std::string("Applying filter: ") + filter->name();
    std::vector<MutationPoint *> tmp;
    TaskExecutor<MutationFilterTask> filterRunner(label, mutations, tmp,
                                                  std::move(tasks));
    filterRunner.execute();
    mutations = std::move(tmp);
  }

  return mutations;
}

std::vector<std::unique_ptr<MutationResult>>
Driver::runMutations(std::vector<MutationPoint *> &mutationPoints) {
  if (mutationPoints.empty()) {
    return std::vector<std::unique_ptr<MutationResult>>();
  }

  if (config.dryRunEnabled) {
    return dryRunMutations(mutationPoints);
  }

  return normalRunMutations(mutationPoints);
}

#pragma mark -

std::vector<std::unique_ptr<MutationResult>>
Driver::dryRunMutations(const std::vector<MutationPoint *> &mutationPoints) {
  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  std::vector<DryRunMutantExecutionTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(DryRunMutantExecutionTask());
  }
  metrics.beginMutantsExecution();
  TaskExecutor<DryRunMutantExecutionTask> mutantRunner(
      "Running mutants (dry run)", mutationPoints, mutationResults,
      std::move(tasks));
  mutantRunner.execute();
  metrics.endMutantsExecution();

  return mutationResults;
}

std::vector<std::unique_ptr<MutationResult>>
Driver::normalRunMutations(const std::vector<MutationPoint *> &mutationPoints) {
  std::vector<std::string> mutatedFunctions;

  SingleTaskExecutor prepareMutations("Prepare mutations", [&]() {
    for (auto point : mutationPoints) {
      point->getBitcode()->addMutation(point);
    }
  });
  prepareMutations.execute();

  auto workers = config.parallelization.workers;
  TaskExecutor<CloneMutatedFunctionsTask> cloneFunctions(
      "Cloning functions for mutation", program.bitcode(), mutatedFunctions,
      std::move(std::vector<CloneMutatedFunctionsTask>(workers)));
  cloneFunctions.execute();

  std::vector<int> Nothing;
  TaskExecutor<DeleteOriginalFunctionsTask> deleteOriginalFunctions(
      "Removing original functions", program.bitcode(), Nothing,
      std::move(std::vector<DeleteOriginalFunctionsTask>(workers)));
  deleteOriginalFunctions.execute();

  TaskExecutor<InsertMutationTrampolinesTask> redirectFunctions(
      "Redirect mutated functions", program.bitcode(), Nothing,
      std::move(std::vector<InsertMutationTrampolinesTask>(workers)));
  redirectFunctions.execute();

  TaskExecutor<ApplyMutationTask> applyMutations(
      "Applying mutations", mutationPoints, Nothing, {ApplyMutationTask()});
  applyMutations.execute();

  std::vector<OriginalCompilationTask> compilationTasks;
  compilationTasks.reserve(workers);
  for (int i = 0; i < workers; i++) {
    compilationTasks.emplace_back(toolchain);
  }
  TaskExecutor<OriginalCompilationTask> mutantCompiler(
      "Compiling original code", program.bitcode(), ownedObjectFiles,
      std::move(compilationTasks));
  mutantCompiler.execute();

  std::vector<object::ObjectFile *> objectFiles;
  for (auto &object : ownedObjectFiles) {
    objectFiles.push_back(object.getBinary());
  }
  for (auto &object : program.precompiledObjectFiles()) {
    objectFiles.push_back(object.getBinary());
  }

  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  std::vector<MutantExecutionTask> tasks;
  tasks.reserve(config.parallelization.mutantExecutionWorkers);
  for (int i = 0; i < config.parallelization.mutantExecutionWorkers; i++) {
    tasks.emplace_back(sandbox, program, testFramework.runner(), config, filter,
                       toolchain.mangler(), objectFiles, mutatedFunctions);
  }
  metrics.beginMutantsExecution();
  TaskExecutor<MutantExecutionTask> mutantRunner(
      "Running mutants", mutationPoints, mutationResults, std::move(tasks));
  mutantRunner.execute();
  metrics.endMutantsExecution();

  return mutationResults;
}

std::vector<llvm::object::ObjectFile *> Driver::AllInstrumentedObjectFiles() {
  std::vector<llvm::object::ObjectFile *> objects;

  for (auto &ownedObject : instrumentedObjectFiles) {
    objects.push_back(ownedObject.getBinary());
  }

  for (auto &ownedObject : program.precompiledObjectFiles()) {
    objects.push_back(ownedObject.getBinary());
  }

  return objects;
}

Driver::Driver(const Configuration &config, const ProcessSandbox &sandbox,
               Program &program, Toolchain &t,
               std::vector<MutationFilter *> &mutationFilters, Filter &f,
               MutationsFinder &mutationsFinder, Metrics &metrics,
               TestFramework &testFramework)
    : config(config), sandbox(sandbox), program(program),
      testFramework(testFramework), toolchain(t),
      mutationFilters(mutationFilters), filter(f),
      mutationsFinder(mutationsFinder), instrumentation(), metrics(metrics) {

  if (config.diagnostics != Diagnostics::None) {
    this->diagnostics = new NormalIDEDiagnostics(config.diagnostics);
  } else {
    this->diagnostics = new NullIDEDiagnostics();
  }
}

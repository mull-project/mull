#include "mull/Driver.h"

#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/Filters.h"
#include "mull/Filters/FunctionFilter.h"
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

#include <algorithm>
#include <sstream>
#include <vector>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;

Driver::~Driver() {
  delete this->ideDiagnostics;
}

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

  return make_unique<Result>(
      std::move(tests), std::move(mutationResults), std::move(filteredMutations));
}

void Driver::compileInstrumentedBitcodeFiles() {
  metrics.beginInstrumentedCompilation();

  for (auto &bitcode : program.bitcode()) {
    instrumentation.recordFunctions(bitcode->getModule());
  }

  std::vector<InstrumentedCompilationTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(diagnostics, instrumentation, toolchain);
  }

  TaskExecutor<InstrumentedCompilationTask> compiler(diagnostics,
                                                     "Compiling instrumented code",
                                                     program.bitcode(),
                                                     instrumentedObjectFiles,
                                                     tasks);
  compiler.execute();

  metrics.endInstrumentedCompilation();
}

void Driver::loadDynamicLibraries() {
  SingleTaskExecutor task(diagnostics, "Loading dynamic libraries", [&]() {
    for (const std::string &dylibPath : program.getDynamicLibraryPaths()) {
      std::string msg;
      auto error = sys::DynamicLibrary::LoadLibraryPermanently(dylibPath.c_str(), &msg);
      if (error) {
        std::stringstream message;
        message << "Cannot load dynamic library '" << dylibPath << "': " << msg << "\n";
        diagnostics.warning(message.str());
      }
    }
  });
  task.execute();
}

std::vector<Test> Driver::findTests() {
  std::vector<Test> tests;
  SingleTaskExecutor task(
      diagnostics, "Searching tests", [&]() { tests = testFramework.finder().findTests(program); });
  task.execute();
  return tests;
}

std::vector<MutationPoint *> Driver::findMutationPoints(vector<Test> &tests) {
  if (tests.empty()) {
    return std::vector<MutationPoint *>();
  }

  auto objectFiles = AllInstrumentedObjectFiles();
  JITEngine jit(diagnostics);

  SingleTaskExecutor prepareOriginalTestRunTask(diagnostics, "Preparing original test run", [&]() {
    testFramework.runner().loadInstrumentedProgram(objectFiles, instrumentation, jit);
  });
  prepareOriginalTestRunTask.execute();

  std::vector<OriginalTestExecutionTask> tasks;
  tasks.reserve(config.parallelization.testExecutionWorkers);
  for (int i = 0; i < config.parallelization.testExecutionWorkers; i++) {
    tasks.emplace_back(
        diagnostics, instrumentation, program, sandbox, testFramework.runner(), config, jit);
  }

  std::vector<std::unique_ptr<ReachableFunction>> reachableFunctions;
  TaskExecutor<OriginalTestExecutionTask> testRunner(
      diagnostics, "Running original tests", tests, reachableFunctions, tasks);
  testRunner.execute();

  std::vector<FunctionUnderTest> functionsUnderTest = mergeReachableFunctions(reachableFunctions);

  std::vector<FunctionUnderTest> filteredFunctions = filterFunctions(functionsUnderTest);

  selectInstructions(filteredFunctions);

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, program, filteredFunctions);

  {
    /// Cleans up the memory allocated for the vector itself as well
    std::vector<OwningBinary<ObjectFile>>().swap(instrumentedObjectFiles);
  }

  return mutationPoints;
}

std::vector<MutationPoint *> Driver::filterMutations(std::vector<MutationPoint *> mutationPoints) {
  std::vector<MutationPoint *> mutations = std::move(mutationPoints);

  for (auto filter : filters.mutationFilters) {
    std::vector<MutationFilterTask> tasks;
    tasks.reserve(config.parallelization.workers);
    for (int i = 0; i < config.parallelization.workers; i++) {
      tasks.emplace_back(*filter);
    }

    std::string label = std::string("Applying filter: ") + filter->name();
    std::vector<MutationPoint *> tmp;
    TaskExecutor<MutationFilterTask> filterRunner(
        diagnostics, label, mutations, tmp, std::move(tasks));
    filterRunner.execute();
    mutations = std::move(tmp);
  }

  return mutations;
}

std::vector<FunctionUnderTest> Driver::filterFunctions(std::vector<FunctionUnderTest> functions) {
  std::vector<FunctionUnderTest> filteredFunctions(std::move(functions));

  for (auto filter : filters.functionFilters) {
    std::vector<FunctionFilterTask> tasks;
    tasks.reserve(config.parallelization.workers);
    for (int i = 0; i < config.parallelization.workers; i++) {
      tasks.emplace_back(*filter);
    }

    std::string label = std::string("Applying function filter: ") + filter->name();
    std::vector<FunctionUnderTest> tmp;
    TaskExecutor<FunctionFilterTask> filterRunner(
        diagnostics, label, filteredFunctions, tmp, std::move(tasks));
    filterRunner.execute();
    filteredFunctions = std::move(tmp);
  }

  return filteredFunctions;
}

void Driver::selectInstructions(std::vector<FunctionUnderTest> &functions) {
  std::vector<InstructionSelectionTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(filters.instructionFilters);
  }

  std::vector<int> Nothing;
  TaskExecutor<InstructionSelectionTask> filterRunner(
      diagnostics, "Instruction selection", functions, Nothing, std::move(tasks));
  filterRunner.execute();
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
      diagnostics, "Running mutants (dry run)", mutationPoints, mutationResults, std::move(tasks));
  mutantRunner.execute();
  metrics.endMutantsExecution();

  return mutationResults;
}

std::vector<std::unique_ptr<MutationResult>>
Driver::normalRunMutations(const std::vector<MutationPoint *> &mutationPoints) {
  std::vector<std::string> mutatedFunctions;

  SingleTaskExecutor prepareMutations(diagnostics, "Prepare mutations", [&]() {
    for (auto point : mutationPoints) {
      point->getBitcode()->addMutation(point);
    }
  });
  prepareMutations.execute();

  auto workers = config.parallelization.workers;
  TaskExecutor<CloneMutatedFunctionsTask> cloneFunctions(
      diagnostics,
      "Cloning functions for mutation",
      program.bitcode(),
      mutatedFunctions,
      std::move(std::vector<CloneMutatedFunctionsTask>(workers)));
  cloneFunctions.execute();

  std::vector<int> Nothing;
  TaskExecutor<DeleteOriginalFunctionsTask> deleteOriginalFunctions(
      diagnostics,
      "Removing original functions",
      program.bitcode(),
      Nothing,
      std::move(std::vector<DeleteOriginalFunctionsTask>(workers)));
  deleteOriginalFunctions.execute();

  TaskExecutor<InsertMutationTrampolinesTask> redirectFunctions(
      diagnostics,
      "Redirect mutated functions",
      program.bitcode(),
      Nothing,
      std::move(std::vector<InsertMutationTrampolinesTask>(workers)));
  redirectFunctions.execute();

  TaskExecutor<ApplyMutationTask> applyMutations(
      diagnostics, "Applying mutations", mutationPoints, Nothing, { ApplyMutationTask() });
  applyMutations.execute();

  std::vector<OriginalCompilationTask> compilationTasks;
  compilationTasks.reserve(workers);
  for (int i = 0; i < workers; i++) {
    compilationTasks.emplace_back(toolchain);
  }
  TaskExecutor<OriginalCompilationTask> mutantCompiler(diagnostics,
                                                       "Compiling original code",
                                                       program.bitcode(),
                                                       ownedObjectFiles,
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
    tasks.emplace_back(diagnostics,
                       sandbox,
                       program,
                       testFramework.runner(),
                       config,
                       toolchain.mangler(),
                       objectFiles,
                       mutatedFunctions);
  }
  metrics.beginMutantsExecution();
  TaskExecutor<MutantExecutionTask> mutantRunner(
      diagnostics, "Running mutants", mutationPoints, mutationResults, std::move(tasks));
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

Driver::Driver(Diagnostics &diagnostics, const Configuration &config, const ProcessSandbox &sandbox,
               Program &program, Toolchain &t, Filters &filters, MutationsFinder &mutationsFinder,
               Metrics &metrics, TestFramework &testFramework)
    : config(config), sandbox(sandbox), program(program), testFramework(testFramework),
      toolchain(t), filters(filters), mutationsFinder(mutationsFinder), diagnostics(diagnostics),
      instrumentation(), metrics(metrics) {

  if (config.diagnostics != IDEDiagnosticsKind::None) {
    this->ideDiagnostics = new NormalIDEDiagnostics(config.diagnostics);
  } else {
    this->ideDiagnostics = new NullIDEDiagnostics();
  }
}

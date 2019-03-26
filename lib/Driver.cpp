#include "Driver.h"

#include "Config/Configuration.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "Result.h"
#include "Testee.h"
#include "MutationResult.h"
#include "TestFrameworks/TestFramework.h"
#include "MutationsFinder.h"
#include "Metrics/Metrics.h"
#include "JunkDetection/JunkDetector.h"
#include "Toolchain/JITEngine.h"
#include "Parallelization/Parallelization.h"
#include "Program/Program.h"

#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <fstream>
#include <vector>
#include <sys/mman.h>
#include <sys/types.h>
#include <map>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;

Driver::~Driver() {
  delete this->sandbox;
  delete this->diagnostics;
}

/// TODO: Remove the following comments as they are irrelevant
/// Populate mull::Context with modules using
/// ModulePaths from mull::Config.
/// mull::Context should be populated using ModuleLoader
/// so that we could inject modules from string for testing purposes

/// Having mull::Context in place we could instantiate TestFinder and find all tests
/// Using same TestFinder we could find mutation points, apply them sequentially and
/// run tests/mutants using newly created TestRunner

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
  auto nonJunkMutationPoints = filterOutJunkMutations(std::move(mutationPoints));
  auto mutationResults = runMutations(nonJunkMutationPoints);

  return make_unique<Result>(std::move(tests),
                             std::move(mutationResults),
                             std::move(nonJunkMutationPoints));
}

void Driver::compileInstrumentedBitcodeFiles() {
  metrics.beginInstrumentedCompilation();

  for (auto &ownedModule : program.modules()) {
    MullModule &module = *ownedModule;
    instrumentation.recordFunctions(module.getModule());
  }

  std::vector<InstrumentedCompilationTask> tasks;
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(instrumentation, toolchain);
  }

  TaskExecutor<InstrumentedCompilationTask> compiler("Compiling instrumented code",
                                                     program.modules(),
                                                     instrumentedObjectFiles,
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

std::vector<std::unique_ptr<Test>> Driver::findTests() {
  std::vector<std::unique_ptr<Test>> tests;
  SingleTaskExecutor task("Searching tests", [&] () {
    tests = testFramework.finder().findTests(program, filter);
  });
  task.execute();
  return tests;
}

std::vector<MutationPoint *>
Driver::findMutationPoints(std::vector<std::unique_ptr<Test>> &tests) {
  if (tests.empty()) {
    return std::vector<MutationPoint *>();
  }

  auto objectFiles = AllInstrumentedObjectFiles();
  JITEngine jit;

  SingleTaskExecutor prepareOriginalTestRunTask("Preparing original test run", [&] () {
    testFramework.runner().loadInstrumentedProgram(objectFiles, instrumentation, jit);
  });
  prepareOriginalTestRunTask.execute();

  std::vector<OriginalTestExecutionTask> tasks;
  for (int i = 0; i < config.parallelization.testExecutionWorkers; i++) {
    tasks.emplace_back(instrumentation, *sandbox, testFramework.runner(), config, filter, jit);
  }

  std::vector<std::unique_ptr<Testee>> testees;
  TaskExecutor<OriginalTestExecutionTask> testRunner("Running original tests", tests, testees, tasks);
  testRunner.execute();

  auto mergedTestees = mergeTestees(testees);
  std::vector<MutationPoint *> mutationPoints = mutationsFinder.getMutationPoints(program, mergedTestees, filter);

  {
    /// Cleans up the memory allocated for the vector itself as well
    std::vector<OwningBinary<ObjectFile>>().swap(instrumentedObjectFiles);
  }

  return mutationPoints;
}

std::vector<MutationPoint *>
Driver::filterOutJunkMutations(std::vector<MutationPoint *> mutationPoints) {
  std::vector<MutationPoint *> nonJunkMutationPoints;
  if (config.junkDetectionEnabled) {
    std::vector<JunkDetectionTask> tasks;
    for (int i = 0; i < config.parallelization.workers; i++) {
      tasks.emplace_back(junkDetector);
    }
    TaskExecutor<JunkDetectionTask> mutantRunner("Filtering out junk mutations", mutationPoints, nonJunkMutationPoints, std::move(tasks));
    mutantRunner.execute();
  } else {
    mutationPoints.swap(nonJunkMutationPoints);
  }

  return nonJunkMutationPoints;
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
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(DryRunMutantExecutionTask());
  }
  metrics.beginMutantsExecution();
  TaskExecutor<DryRunMutantExecutionTask> mutantRunner("Running mutants (dry run)", mutationPoints, mutationResults, std::move(tasks));
  mutantRunner.execute();
  metrics.endMutantsExecution();

  return mutationResults;
}

std::vector<std::unique_ptr<MutationResult>> Driver::normalRunMutations(const std::vector<MutationPoint *> &mutationPoints) {
  std::vector<std::string> mutatedFunctions;

  SingleTaskExecutor prepareMutationsTask("Preparing mutations", [&] () {
    for (auto &module : program.modules()) {
      auto functions = module->prepareMutations();
      for (auto &name : functions) {
        mutatedFunctions.push_back(name);
      }
    }
  });
  prepareMutationsTask.execute();

  std::vector<int> empty;
  TaskExecutor<ApplyMutationTask> applyMutations("Applying mutations", mutationPoints, empty, {ApplyMutationTask()});
  applyMutations.execute();

  std::vector<OriginalCompilationTask> compilationTasks;
  for (int i = 0; i < config.parallelization.workers; i++) {
    compilationTasks.emplace_back(toolchain);
  }
  TaskExecutor<OriginalCompilationTask> mutantCompiler("Compiling original code", program.modules(), ownedObjectFiles, std::move(compilationTasks));
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
  for (int i = 0; i < config.parallelization.mutantExecutionWorkers; i++) {
    tasks.emplace_back(*sandbox, testFramework.runner(), config, filter, toolchain.mangler(), objectFiles, mutatedFunctions);
  }
  metrics.beginMutantsExecution();
  TaskExecutor<MutantExecutionTask> mutantRunner("Running mutants", mutationPoints, mutationResults, std::move(tasks));
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

Driver::Driver(const Configuration &config,
               Program &program,
               TestFramework &testFramework,
               Toolchain &t,
               Filter &f,
               MutationsFinder &mutationsFinder,
               Metrics &metrics,
               JunkDetector &junkDetector)
    : config(config), program(program), testFramework(testFramework), toolchain(t), filter(f), mutationsFinder(mutationsFinder),
      instrumentation(), metrics(metrics), junkDetector(junkDetector) {

  if (config.forkEnabled) {
    this->sandbox = new ForkProcessSandbox();
  } else {
    this->sandbox = new NullProcessSandbox();
  }


  if (config.diagnostics != Diagnostics::None) {
    this->diagnostics = new NormalIDEDiagnostics(config.diagnostics);
  } else {
    this->diagnostics = new NullIDEDiagnostics();
  }
}

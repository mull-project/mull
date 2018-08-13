#include "Driver.h"

#include "Config.h"
#include "Context.h"
#include "Logger.h"
#include "ModuleLoader.h"
#include "Result.h"
#include "Testee.h"
#include "MutationResult.h"
#include "TestFinder.h"
#include "TestRunner.h"
#include "MutationsFinder.h"
#include "Metrics/Metrics.h"
#include "JunkDetection/JunkDetector.h"
#include "Toolchain/JITEngine.h"
#include "Parallelization/Parallelization.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>

#include <algorithm>
#include <fstream>
#include <vector>
#include <sys/mman.h>
#include <sys/types.h>


using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;

class InstrumentedCompilationTask {
public:
  using In = std::vector<std::unique_ptr<MullModule>>;
  using Out = std::vector<object::OwningBinary<object::ObjectFile>>;
  using iterator = In::const_iterator;

  InstrumentedCompilationTask(Instrumentation &instrumentation, Toolchain &toolchain)
    : instrumentation(instrumentation), toolchain(toolchain) {}

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter) {
    EngineBuilder builder;
    auto target = builder.selectTarget(llvm::Triple(), "", "",
                                       llvm::SmallVector<std::string, 1>());
    std::unique_ptr<TargetMachine> localMachine(target);

    for (auto it = begin; it != end; it++) {
      auto &module = *it->get();
      auto objectFile = toolchain.cache().getInstrumentedObject(module);
      if (objectFile.getBinary() == nullptr) {
        LLVMContext instrumentationContext;
        auto clonedModule = module.clone(instrumentationContext);

        instrumentation.insertCallbacks(clonedModule->getModule());
        objectFile = toolchain.compiler().compileModule(*clonedModule, *localMachine);
        toolchain.cache().putInstrumentedObject(objectFile, module);
      }
      storage.push_back(std::move(objectFile));
      counter.increment();
    }
  }
  Instrumentation &instrumentation;
  Toolchain &toolchain;
};

class MutantCompilationTask {
public:
  using In = std::vector<std::unique_ptr<MullModule>>;
  using Out = std::vector<object::OwningBinary<object::ObjectFile>>;
  using iterator = In::const_iterator;

  MutantCompilationTask(Toolchain &toolchain) : toolchain(toolchain) {}

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter) {
    EngineBuilder builder;
    auto target = builder.selectTarget(llvm::Triple(), "", "",
                                       llvm::SmallVector<std::string, 1>());
    std::unique_ptr<TargetMachine> localMachine(target);

    for (auto it = begin; it != end; it++) {
      auto &module = *it->get();

      auto objectFile = toolchain.cache().getObject(module);
      if (objectFile.getBinary() == nullptr) {
        LLVMContext localContext;
        auto clonedModule = module.clone(localContext);
        objectFile = toolchain.compiler().compileModule(*clonedModule.get());
        toolchain.cache().putObject(objectFile, module);
      }

      storage.push_back(std::move(objectFile));
      counter.increment();
    }
  }
  Toolchain &toolchain;
};

class OriginalTestExecutionTask {
public:
  using In = std::vector<std::unique_ptr<Test>>;
  using Out = std::vector<std::unique_ptr<Testee>>;
  using iterator = In::const_iterator;

  OriginalTestExecutionTask(Instrumentation &instrumentation,
                            ProcessSandbox &sandbox,
                            TestRunner &runner,
                            Config &config,
                            Filter &filter,
                            JITEngine &jit)
    : instrumentation(instrumentation),
      sandbox(sandbox),
      runner(runner),
      config(config),
      filter(filter),
      jit(jit) {}

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter) {
    for (auto it = begin; it != end; ++it, counter.increment()) {
      auto &test = *it;

      instrumentation.setupInstrumentationInfo(test.get());

      ExecutionResult testExecutionResult = sandbox.run([&]() {
        return runner.runTest(test.get(), jit);
      }, config.getTimeout());

      test->setExecutionResult(testExecutionResult);

      std::vector<std::unique_ptr<Testee>> testees;

      if (testExecutionResult.status == Passed) {
        testees = instrumentation.getTestees(test.get(), filter, config.getMaxDistance());
      }
      instrumentation.cleanupInstrumentationInfo(test.get());

      if (testees.empty()) {
        continue;
      }

      for (auto it = std::next(testees.begin()); it != testees.end(); ++it) {
        storage.push_back(std::move(*it));
      }
    }
  }
  Instrumentation &instrumentation;
  ProcessSandbox &sandbox;
  TestRunner &runner;
  Config &config;
  Filter &filter;
  JITEngine &jit;
};

class MutantExecutionTask {
public:
  using In = const std::vector<MutationPoint *>;
  using Out = std::vector<std::unique_ptr<MutationResult>>;
  using iterator = In::const_iterator;

  MutantExecutionTask(Driver &driver,
                      ProcessSandbox &sandbox,
                      TestRunner &runner,
                      Config &config,
                      Toolchain &toolchain,
                      Filter &filter)
    : sandbox(sandbox), runner(runner),
      config(config), toolchain(toolchain), filter(filter), driver(driver) {}

  void operator() (iterator begin, iterator end, Out &storage, progress_counter &counter) {
    EngineBuilder builder;
    auto target = builder.selectTarget(llvm::Triple(), "", "",
                                       llvm::SmallVector<std::string, 1>());
    std::unique_ptr<TargetMachine> localMachine(target);

    for (auto it = begin; it != end; ++it, counter.increment()) {
      auto mutationPoint = *it;
      auto objectFilesWithMutant = driver.AllButOne(mutationPoint->getOriginalModule()->getModule());

      auto mutant = toolchain.cache().getObject(*mutationPoint);
      if (mutant.getBinary() == nullptr) {
        LLVMContext localContext;
        auto clonedModule = mutationPoint->getOriginalModule()->clone(localContext);
        mutationPoint->applyMutation(*clonedModule.get());
        mutant = toolchain.compiler().compileModule(*clonedModule.get(), *localMachine);
        toolchain.cache().putObject(mutant, *mutationPoint);
      }

      objectFilesWithMutant.push_back(mutant.getBinary());

      runner.loadProgram(objectFilesWithMutant, jit);

      auto atLeastOneTestFailed = false;
      for (auto &reachableTest : mutationPoint->getReachableTests()) {
        auto test = reachableTest.first;
        auto distance = reachableTest.second;

        ExecutionResult result;
        if (config.failFastModeEnabled() && atLeastOneTestFailed) {
          result.status = ExecutionStatus::FailFast;
        } else {
          const auto timeout = test->getExecutionResult().runningTime * 10;
          const auto sandboxTimeout = std::max(30LL, timeout);

          result = sandbox.run([&]() {
            ExecutionStatus status = runner.runTest(test, jit);
            assert(status != ExecutionStatus::Invalid && "Expect to see valid TestResult");
            return status;
          }, sandboxTimeout);

          assert(result.status != ExecutionStatus::Invalid &&
                 "Expect to see valid TestResult");

          if (result.status != ExecutionStatus::Passed) {
            atLeastOneTestFailed = true;
          }
        }

        storage.push_back(make_unique<MutationResult>(result, mutationPoint, distance, test));
      }

      objectFilesWithMutant.pop_back();
    }

  }
  JITEngine jit;
  ProcessSandbox &sandbox;
  TestRunner &runner;
  Config &config;
  Toolchain &toolchain;
  Filter &filter;
  Driver &driver;
};

Driver::~Driver() {
  delete this->sandbox;
  delete this->diagnostics;
}

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
  loadBitcodeFilesIntoMemory();
  compileInstrumentedBitcodeFiles();
  loadPrecompiledObjectFiles();
  loadDynamicLibraries();

  auto tests = findTests();
  auto mutationPoints = findMutationPoints(tests);
  auto nonJunkMutationPoints = filterOutJunkMutations(std::move(mutationPoints));
  auto mutationResults = runMutations(nonJunkMutationPoints);

  return make_unique<Result>(std::move(tests),
                             std::move(mutationResults),
                             std::move(nonJunkMutationPoints));
}

void Driver::loadBitcodeFilesIntoMemory() {
  metrics.beginLoadModules();
  std::vector<std::string> bitcodePaths = config.getBitcodePaths();
  std::vector<unique_ptr<MullModule>> modules =
    loader.loadModulesFromBitcodeFileList(bitcodePaths);
  metrics.endLoadModules();

  for (auto &ownedModule : modules) {
    assert(ownedModule && "Can't load module");
    context.addModule(std::move(ownedModule));
  }
}

void Driver::compileInstrumentedBitcodeFiles() {
  metrics.beginInstrumentedCompilation();

  for (auto &ownedModule : context.getModules()) {
    MullModule &module = *ownedModule;
    instrumentation.recordFunctions(module.getModule());
  }

  int workers = std::thread::hardware_concurrency();
  std::vector<InstrumentedCompilationTask> tasks;
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(instrumentation, toolchain);
  }

  TaskExecutor<InstrumentedCompilationTask> compiler("Compiling instrumented code",
                                                     context.getModules(),
                                                     instrumentedObjectFiles,
                                                     tasks);
  compiler.execute();

  metrics.endInstrumentedCompilation();
}

void Driver::loadPrecompiledObjectFiles() {
  metrics.beginLoadPrecompiledObjectFiles();
  for (std::string &objectFilePath: config.getObjectFilesPaths()) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
    MemoryBuffer::getFile(objectFilePath);

    if (!buffer) {
      Logger::error() << "Cannot load object file: " << objectFilePath << "\n";
      continue;
    }

    Expected<std::unique_ptr<ObjectFile>> objectOrError =
    ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

    if (!objectOrError) {
      Logger::error() << "Cannot create object file: " << objectFilePath << "\n";
      continue;
    }

    std::unique_ptr<ObjectFile> objectFile(std::move(objectOrError.get()));

    auto owningObject = OwningBinary<ObjectFile>(std::move(objectFile),
                                                 std::move(buffer.get()));
    precompiledObjectFiles.push_back(std::move(owningObject));
  }
  metrics.endLoadPrecompiledObjectFiles();
}

void Driver::loadDynamicLibraries() {
  metrics.beginLoadDynamicLibraries();
  for (std::string &dylibPath: config.getDynamicLibrariesPaths()) {
    sys::DynamicLibrary::LoadLibraryPermanently(dylibPath.c_str());
  }
  metrics.endLoadDynamicLibraries();
}

std::vector<std::unique_ptr<Test>> Driver::findTests() {
  metrics.beginFindTests();
  auto tests = finder.findTests(context, filter);
  metrics.endFindTests();

  Logger::debug() << "Found " << tests.size() << " tests\n";

  return tests;
}

std::vector<MutationPoint *>
Driver::findMutationPoints(std::vector<std::unique_ptr<Test>> &tests) {
  if (tests.empty()) {
    return std::vector<MutationPoint *>();
  }

  Logger::debug() << "Running tests and searching mutations\n";

  auto objectFiles = AllInstrumentedObjectFiles();
  JITEngine jit;

  JITEngine jit;

  metrics.beginLoadOriginalProgram();
  runner.loadInstrumentedProgram(objectFiles, instrumentation, jit);
  metrics.endLoadOriginalProgram();

  int workers = std::thread::hardware_concurrency();
  std::vector<OriginalTestExecutionTask> tasks;
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(instrumentation, *sandbox, runner, config, filter, jit);
  }

  metrics.beginOriginalTestExecution();
  std::vector<std::unique_ptr<Testee>> testees;
  TaskExecutor<OriginalTestExecutionTask> testRunner("Running original tests", tests, testees, tasks);
  testRunner.execute();
  metrics.endOriginalTestExecution();

  for (auto &testee : testees) {
    auto points = mutationsFinder.getMutationPoints(context, *testee, filter);
    std::copy(points.begin(), points.end(), std::back_inserter(mutationPoints));
  }

  auto mergedTestees = mergeTestees(allTestees);
  std::vector<MutationPoint *> mutationPoints = mutationsFinder.getMutationPoints(context, mergedTestees, filter);

  {
    /// Cleans up the memory allocated for the vector itself as well
    std::vector<OwningBinary<ObjectFile>>().swap(instrumentedObjectFiles);
  }

  Logger::debug() << "Found " << mutationPoints.size() << " mutations\n";

  return mutationPoints;
}

std::vector<MutationPoint *>
Driver::filterOutJunkMutations(std::vector<MutationPoint *> mutationPoints) {
  std::vector<MutationPoint *> nonJunkMutationPoints;
  if (config.junkDetectionEnabled()) {
    for (auto point: mutationPoints) {
      if (!junkDetector.isJunk(point)) {
        nonJunkMutationPoints.push_back(point);
      }
    }
    auto junkSize = mutationPoints.size() - nonJunkMutationPoints.size();
    Logger::debug() << "Filtered out " << junkSize << " junk mutations\n";
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

  if (config.dryRunModeEnabled()) {
    return dryRunMutations(mutationPoints);
  }

  return normalRunMutations(mutationPoints);
}

#pragma mark -

std::vector<std::unique_ptr<MutationResult>>
Driver::dryRunMutations(const std::vector<MutationPoint *> &mutationPoints) {
  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  const auto mutationsCount = mutationPoints.size();
  auto mutantIndex = 1;

  for (auto mutationPoint : mutationPoints) {
    Logger::debug() << "[" << mutantIndex++ << "/" << mutationsCount << "]: "  << mutationPoint->getUniqueIdentifier() << "\n";

    auto testsCount = mutationPoint->getReachableTests().size();
    auto testIndex = 1;

    for (auto &reachableTest : mutationPoint->getReachableTests()) {
      auto test = reachableTest.first;
      auto distance = reachableTest.second;

      Logger::debug().indent(2) << "[" << testIndex++ << "/" << testsCount << "] " << test->getTestDisplayName() << ": ";

      auto timeout = test->getExecutionResult().runningTime * 10;

      ExecutionResult result;
      result.status = DryRun;
      result.runningTime = timeout;

      Logger::debug() << result.getStatusAsString() << "\n";

      mutationResults.push_back(make_unique<MutationResult>(result, mutationPoint, distance, test));
    }
  }

  return mutationResults;
}

std::vector<std::unique_ptr<MutationResult>> Driver::normalRunMutations(const std::vector<MutationPoint *> &mutationPoints) {
  std::vector<MutantCompilationTask> compilationTasks;
  int compilationWorkers = std::thread::hardware_concurrency();
  for (int i = 0; i < compilationWorkers; i++) {
    compilationTasks.emplace_back(toolchain);
  }
  TaskExecutor<MutantCompilationTask> mutantCompiler("Compiling original code", context.getModules(), ownedObjectFiles, std::move(compilationTasks));
  mutantCompiler.execute();

  for (size_t i = 0; i < ownedObjectFiles.size(); i++) {
    auto &module = context.getModules().at(i);
    auto &objectFile = ownedObjectFiles.at(i);
    innerCache.insert(std::make_pair(module->getModule(), objectFile.getBinary()));
  }

  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  std::vector<MutantExecutionTask> tasks;
  int workers = std::thread::hardware_concurrency();
  for (int i = 0; i < workers; i++) {
    tasks.emplace_back(*this, *sandbox, runner, config, toolchain, filter);
  }
  metrics.beginMutantsExecution();
  TaskExecutor<MutantExecutionTask> mutantRunner("Running mutants", mutationPoints, mutationResults, std::move(tasks));
  mutantRunner.execute();
  metrics.endMutantsExecution();

  return mutationResults;
}

std::vector<llvm::object::ObjectFile *> Driver::AllButOne(llvm::Module *One) {
  std::vector<llvm::object::ObjectFile *> Objects;

  for (auto &CachedEntry : innerCache) {
    if (One != CachedEntry.first) {
      Objects.push_back(CachedEntry.second);
    }
  }

  for (OwningBinary<ObjectFile> &object: precompiledObjectFiles) {
    Objects.push_back(object.getBinary());
  }

  return Objects;
}

std::vector<llvm::object::ObjectFile *> Driver::AllInstrumentedObjectFiles() {
  std::vector<llvm::object::ObjectFile *> objects;

  for (auto &ownedObject : instrumentedObjectFiles) {
    objects.push_back(ownedObject.getBinary());
  }

  for (auto &ownedObject : precompiledObjectFiles) {
    objects.push_back(ownedObject.getBinary());
  }

  return objects;
}

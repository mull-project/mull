#include "mull/Driver.h"

#include "mull/BitcodeMetadataReader.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/FilePathFilter.h"
#include "mull/Filters/Filters.h"
#include "mull/Filters/FunctionFilter.h"
#include "mull/Filters/GitDiffFilter.h"
#include "mull/Filters/JunkMutationFilter.h"
#include "mull/Filters/NoDebugInfoFilter.h"
#include "mull/FunctionUnderTest.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/Mutant.h"
#include "mull/MutantRunner.h"
#include "mull/MutationResult.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"
#include "mull/Result.h"
#include "mull/Toolchain/Runner.h"

#include <llvm/ProfileData/Coverage/CoverageMapping.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;

Driver::~Driver() {
  delete this->ideDiagnostics;
}

std::unique_ptr<Result> Driver::run() {
  auto mutationPoints = findMutationPoints();
  auto filteredMutations = filterMutations(std::move(mutationPoints));
  prepareMutations(filteredMutations);
  std::vector<std::unique_ptr<Mutant>> mutants;
  singleTask.execute("Deduplicate mutants", [&]() {
    std::unordered_map<std::string, std::vector<MutationPoint *>> mapping;
    for (MutationPoint *point : filteredMutations) {
      mapping[point->getUserIdentifier()].push_back(point);
    }
    for (auto &pair : mapping) {
      std::string identifier = pair.first;
      MutationPoint *anyPoint = pair.second.front();
      std::string mutatorIdentifier = anyPoint->getMutatorIdentifier();
      const SourceLocation &sourceLocation = anyPoint->getSourceLocation();
      const SourceLocation &endLocation = anyPoint->getEndLocation();
      bool covered = false;
      for (MutationPoint *point : pair.second) {
        /// Consider a mutant covered if at least one of the mutation points is covered
        if (point->isCovered()) {
          covered = true;
          break;
        }
      }

      mutants.push_back(std::make_unique<Mutant>(
          identifier, mutatorIdentifier, sourceLocation, endLocation, covered));
      mutants.back()->setMutatorKind(anyPoint->getMutator()->mutatorKind());
    }
    std::sort(std::begin(mutants), std::end(mutants), MutantComparator());
  });

  auto mutationResults = runMutations(mutants);

  return std::make_unique<Result>(std::move(mutants), std::move(mutationResults));
}

std::vector<MutationPoint *> Driver::findMutationPoints() {
  if (!config.skipSanityCheckRun) {
    Runner runner(diagnostics);
    singleTask.execute("Sanity check run", [&]() {
      ExecutionResult result = runner.runProgram(
          config.executable, {}, {}, config.timeout, config.captureTestOutput, std::nullopt);
      if (result.status != Passed) {
        std::stringstream failureMessage;
        failureMessage << "Original test failed\n";
        failureMessage << "test: ";
        failureMessage << "main"
                       << "\n";
        failureMessage << "status: ";
        failureMessage << result.getStatusAsString() << "\n";
        failureMessage << "stdout: '";
        failureMessage << result.stdoutOutput << "'\n";
        failureMessage << "stderr: '";
        failureMessage << result.stderrOutput << "'\n";
        diagnostics.warning(failureMessage.str());
      }
    });
  }

  std::vector<FunctionUnderTest> functionsUnderTest = getFunctionsUnderTest();
  std::vector<FunctionUnderTest> filteredFunctions = filterFunctions(functionsUnderTest);

  selectInstructions(filteredFunctions);

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, filteredFunctions);

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
Driver::runMutations(std::vector<std::unique_ptr<Mutant>> &mutants) {
  if (mutants.empty()) {
    if (config.keepExecutable && !config.outputFile.empty()) {
      llvm::sys::fs::copy_file(config.executable, config.outputFile);
    }
    diagnostics.info("No mutants. Skipping execution.");
    return std::vector<std::unique_ptr<MutationResult>>();
  }
  if (config.dryRunEnabled) {
    return dryRunMutations(mutants);
  }

  return normalRunMutations(mutants);
}

std::vector<std::unique_ptr<MutationResult>>
Driver::dryRunMutations(std::vector<std::unique_ptr<Mutant>> &mutants) {
  std::vector<std::unique_ptr<MutationResult>> mutationResults;

  std::vector<DryRunMutantExecutionTask> tasks;
  tasks.reserve(config.parallelization.workers);
  for (int i = 0; i < config.parallelization.workers; i++) {
    tasks.emplace_back(DryRunMutantExecutionTask());
  }
  TaskExecutor<DryRunMutantExecutionTask> mutantRunner(
      diagnostics, "Running mutants (dry run)", mutants, mutationResults, std::move(tasks));
  mutantRunner.execute();

  return mutationResults;
}

void Driver::prepareMutations(std::vector<MutationPoint *> mutationPoints) {
  if (config.dryRunEnabled) {
    return;
  }
  singleTask.execute("Prepare mutations", [&]() {
    for (auto point : mutationPoints) {
      point->getBitcode()->addMutation(point);
    }
  });

  auto workers = config.parallelization.workers;
  std::vector<int> devNull;
  TaskExecutor<CloneMutatedFunctionsTask> cloneFunctions(
      diagnostics,
      "Cloning functions for mutation",
      program.bitcode(),
      devNull,
      std::vector<CloneMutatedFunctionsTask>(workers));
  cloneFunctions.execute();

  std::vector<int> Nothing;
  TaskExecutor<DeleteOriginalFunctionsTask> deleteOriginalFunctions(
      diagnostics,
      "Removing original functions",
      program.bitcode(),
      Nothing,
      std::vector<DeleteOriginalFunctionsTask>(workers));
  deleteOriginalFunctions.execute();

  TaskExecutor<InsertMutationTrampolinesTask> redirectFunctions(
      diagnostics,
      "Redirect mutated functions",
      program.bitcode(),
      Nothing,
      std::vector<InsertMutationTrampolinesTask>(workers));
  redirectFunctions.execute();

  TaskExecutor<ApplyMutationTask> applyMutations(
      diagnostics, "Applying mutations", mutationPoints, Nothing, { ApplyMutationTask() });
  applyMutations.execute();
}

std::vector<std::unique_ptr<MutationResult>>
Driver::normalRunMutations(std::vector<std::unique_ptr<Mutant>> &mutants) {
  auto workers = config.parallelization.workers;

  std::vector<OriginalCompilationTask> compilationTasks;
  compilationTasks.reserve(workers);
  for (int i = 0; i < workers; i++) {
    compilationTasks.emplace_back(toolchain);
  }
  std::vector<std::string> objectFiles;
  TaskExecutor<OriginalCompilationTask> mutantCompiler(diagnostics,
                                                       "Compiling original code",
                                                       program.bitcode(),
                                                       objectFiles,
                                                       std::move(compilationTasks));
  mutantCompiler.execute();

  std::string executable;
  singleTask.execute("Link mutated program",
                     [&]() { executable = toolchain.linker().linkObjectFiles(objectFiles); });

  diagnostics.info("Mutated executable: "s + executable);
  if (!config.keepObjectFiles) {
    for (auto &objectFile : objectFiles) {
      llvm::sys::fs::remove(objectFile);
    }
  }

  if (config.mutateOnly) {
    return std::vector<std::unique_ptr<MutationResult>>();
  }

  MutantRunner mutantRunner(diagnostics, config);
  std::vector<std::unique_ptr<MutationResult>> mutationResults =
      mutantRunner.runMutants(executable, mutants);

  if (!config.keepExecutable) {
    llvm::sys::fs::remove(executable);
  }

  return mutationResults;
}

Driver::Driver(Diagnostics &diagnostics, const Configuration &config, Program &program,
               Toolchain &t, Filters &filters, MutationsFinder &mutationsFinder)
    : config(config), program(program), toolchain(t), mutationsFinder(mutationsFinder),
      diagnostics(diagnostics), filters(filters), singleTask(diagnostics) {

  if (config.diagnostics != IDEDiagnosticsKind::None) {
    this->ideDiagnostics = new NormalIDEDiagnostics(config.diagnostics);
  } else {
    this->ideDiagnostics = new NullIDEDiagnostics();
  }
}

static std::unique_ptr<llvm::coverage::CoverageMapping>
loadCoverage(const Configuration &configuration, Diagnostics &diagnostics) {
  if (configuration.coverageInfo.empty()) {
    return nullptr;
  }
  llvm::Expected<std::unique_ptr<llvm::coverage::CoverageMapping>> maybeMapping =
      llvm::coverage::CoverageMapping::load({ configuration.executable },
                                            configuration.coverageInfo);
  if (!maybeMapping) {
    std::string error;
    llvm::raw_string_ostream os(error);
    llvm::logAllUnhandledErrors(maybeMapping.takeError(), os, "Cannot read coverage info: ");
    diagnostics.warning(os.str());
    return nullptr;
  }
  return std::move(maybeMapping.get());
}

std::vector<FunctionUnderTest> Driver::getFunctionsUnderTest() {
  std::vector<FunctionUnderTest> functionsUnderTest;

  singleTask.execute("Gathering functions under test", [&]() {
    std::unique_ptr<llvm::coverage::CoverageMapping> coverage = loadCoverage(config, diagnostics);
    if (coverage) {
      /// Some of the function records contain just name, the others are prefixed with the filename
      /// to avoid collisions
      /// TODO: check case when filename:functionName is not enough to resolve collisions
      /// TODO: pick a proper data structure
      std::unordered_map<
          std::string,
          std::unordered_map<std::string, std::vector<llvm::coverage::CountedRegion>>>
          scopedFunctions;
      std::unordered_map<std::string, std::vector<llvm::coverage::CountedRegion>> unscopedFunctions;
      for (auto &it : coverage->getCoveredFunctions()) {
        if (!it.ExecutionCount) {
          continue;
        }
        std::string scope;
        std::string name = it.Name;
        size_t idx = name.find(':');
        if (idx != std::string::npos) {
          scope = name.substr(0, idx);
          name = name.substr(idx + 1);
        }
        if (scope.empty()) {
          unscopedFunctions[name] = it.CountedRegions;
        } else {
          scopedFunctions[scope][name] = it.CountedRegions;
        }
      }
      for (auto &bitcode : program.bitcode()) {
        for (llvm::Function &function : *bitcode->getModule()) {
          bool covered = false;
          std::vector<llvm::coverage::CountedRegion> linecoverage = {};
          std::string name = function.getName().str();
          if (unscopedFunctions.count(name)) {
            covered = true;
            std::swap(linecoverage, unscopedFunctions[name]);
          } else {
            std::string filepath = SourceLocation::locationFromFunction(&function).unitFilePath;
            std::string scope = llvm::sys::path::filename(filepath).str();
            if (scopedFunctions[scope].count(name)) {
              covered = true;
              std::swap(linecoverage, scopedFunctions[scope][name]);
            }
          }
          if (covered) {
            functionsUnderTest.emplace_back(&function, bitcode.get(), true, linecoverage);
          } else if (config.includeNotCovered) {
            functionsUnderTest.emplace_back(&function, bitcode.get());
          }
        }
      }
    } else {
      if (config.includeNotCovered) {
        diagnostics.warning("-include-not-covered is enabled, but there is no coverage info!");
      }
      for (auto &bitcode : program.bitcode()) {
        for (llvm::Function &function : *bitcode->getModule()) {
          functionsUnderTest.emplace_back(&function, bitcode.get(), true);
        }
      }
    }
  });

  return functionsUnderTest;
}

void mull::mutateBitcode(llvm::Module &module) {
  /// Setup

  Diagnostics diagnostics;
  std::string configPath = Configuration::findConfig(diagnostics);
  Configuration configuration;
  if (configPath.empty()) {
    diagnostics.warning("Mull cannot find config (mull.yml). Using some defaults.");
  } else {
    configuration = Configuration::loadFromDisk(diagnostics, configPath);
  }
  configuration.parallelization.normalize();

  if (configuration.debugEnabled) {
    diagnostics.enableDebugMode();
  }
  if (configuration.quiet) {
    diagnostics.makeQuiet();
  }
  if (configuration.silent) {
    diagnostics.makeSilent();
  }

  if (!configPath.empty()) {
    diagnostics.info("Using configuration "s + configPath);
  }

  std::vector<std::unique_ptr<mull::Filter>> filterStorage;
  mull::Filters filters;

  auto noDebugInfoFilter = new mull::NoDebugInfoFilter;
  auto filePathFilter = new mull::FilePathFilter;

  for (const auto &regex : configuration.excludePaths) {
    auto added = filePathFilter->exclude(regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for exclude-path: '" << regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }
  for (const auto &regex : configuration.includePaths) {
    auto added = filePathFilter->include(regex);
    if (!added.first) {
      std::stringstream warningMessage;
      warningMessage << "Invalid regex for include-path: '" << regex
                     << "' has been ignored. Error: " << added.second;
      diagnostics.warning(warningMessage.str());
    }
  }

  filterStorage.emplace_back(noDebugInfoFilter);
  filterStorage.emplace_back(filePathFilter);

  filters.mutationFilters.push_back(noDebugInfoFilter);
  filters.functionFilters.push_back(noDebugInfoFilter);
  filters.instructionFilters.push_back(noDebugInfoFilter);

  filters.mutationFilters.push_back(filePathFilter);
  filters.functionFilters.push_back(filePathFilter);

  std::string cxxCompilationFlags;
  for (auto &flag : configuration.compilerFlags) {
    cxxCompilationFlags += flag + ' ';
  }
  mull::BitcodeMetadataReader bitcodeCompilationDatabaseLoader;
  std::unordered_map<std::string, std::string> bitcodeCompilationFlags =
      bitcodeCompilationDatabaseLoader.getCompilationDatabase(module);
  if (!bitcodeCompilationFlags.empty()) {
    diagnostics.info(std::string("Found compilation flags in the input bitcode"));
  }

  mull::ASTStorage astStorage(diagnostics,
                              configuration.compilationDatabasePath,
                              cxxCompilationFlags,
                              bitcodeCompilationFlags);

  mull::CXXJunkDetector junkDetector(diagnostics, astStorage);
  if (!configuration.junkDetectionDisabled) {
    auto *junkFilter = new mull::JunkMutationFilter(junkDetector);
    filters.mutationFilters.push_back(junkFilter);
    filterStorage.emplace_back(junkFilter);
  }

  if (!configuration.gitDiffRef.empty()) {
    if (configuration.gitProjectRoot.empty()) {
      std::stringstream debugMessage;
      debugMessage
          << "-git-diff-ref option has been provided but the path to the Git project root has not "
             "been specified via -git-project-root. The incremental testing will be disabled.";
      diagnostics.warning(debugMessage.str());
    } else if (!llvm::sys::fs::is_directory(configuration.gitProjectRoot)) {
      std::stringstream debugMessage;
      debugMessage << "directory provided by -git-project-root does not exist, ";
      debugMessage << "the incremental testing will be disabled: ";
      debugMessage << configuration.gitProjectRoot;
      diagnostics.warning(debugMessage.str());
    } else {
      llvm::SmallString<256> tmpGitProjectRoot;
      std::string gitProjectRoot = configuration.gitProjectRoot;
      if (!llvm::sys::fs::real_path(gitProjectRoot, tmpGitProjectRoot)) {
        gitProjectRoot = tmpGitProjectRoot.str();

        diagnostics.info("Incremental testing using Git Diff is enabled.\n"s + "- Git ref: " +
                         configuration.gitDiffRef + "\n" + "- Git project root: " + gitProjectRoot);
        mull::GitDiffFilter *gitDiffFilter = mull::GitDiffFilter::createFromGitDiff(
            diagnostics, gitProjectRoot, configuration.gitDiffRef);

        if (gitDiffFilter) {
          filterStorage.emplace_back(gitDiffFilter);
          filters.instructionFilters.push_back(gitDiffFilter);
        }
      } else {
        diagnostics.warning("could not expand -git-project-root to an absolute path: "s +
                            gitProjectRoot);
      }
    }
  }

  SingleTaskExecutor singleTask(diagnostics);

  /// Actual Work

  /// How can I check that -g flag (debug info enable) was set, from llvm pass
  /// https://stackoverflow.com/a/21713717/598057
  if (module.getNamedMetadata("llvm.dbg.cu") == nullptr) {
    diagnostics.warning("Mull cannot find debug information. Recompile with `-g` flag.");
  }

  if (bitcodeCompilationFlags.empty()) {
    diagnostics.warning(
        "Mull cannot find compiler flags. Recompile with `-grecord-command-line` flag.");
  }

  Bitcode bitcode(&module);
  std::vector<FunctionUnderTest> functionsUnderTest;
  singleTask.execute("Gathering functions under test", [&]() {
    std::unique_ptr<llvm::coverage::CoverageMapping> coverage =
        loadCoverage(configuration, diagnostics);
    if (coverage) {
      /// Some of the function records contain just name, the others are prefixed with the filename
      /// to avoid collisions
      /// TODO: check case when filename:functionName is not enough to resolve collisions
      /// TODO: pick a proper data structure
      std::unordered_map<
          std::string,
          std::unordered_map<std::string, std::vector<llvm::coverage::CountedRegion>>>
          scopedFunctions;
      std::unordered_map<std::string, std::vector<llvm::coverage::CountedRegion>> unscopedFunctions;
      for (auto &it : coverage->getCoveredFunctions()) {
        if (!it.ExecutionCount) {
          continue;
        }
        std::string scope;
        std::string name = it.Name;
        size_t idx = name.find(':');
        if (idx != std::string::npos) {
          scope = name.substr(0, idx);
          name = name.substr(idx + 1);
        }
        if (scope.empty()) {
          unscopedFunctions[name] = it.CountedRegions;
        } else {
          scopedFunctions[scope][name] = it.CountedRegions;
        }
      }

      for (llvm::Function &function : module) {
        bool covered = false;
        std::vector<llvm::coverage::CountedRegion> linecoverage = {};
        std::string name = function.getName().str();
        if (unscopedFunctions.count(name)) {
          covered = true;
          std::swap(linecoverage, unscopedFunctions[name]);
        } else {
          std::string filepath = SourceLocation::locationFromFunction(&function).unitFilePath;
          std::string scope = llvm::sys::path::filename(filepath).str();
          if (scopedFunctions[scope].count(name)) {
            covered = true;
            std::swap(linecoverage, scopedFunctions[scope][name]);
          }
        }
        if (covered) {
          functionsUnderTest.emplace_back(&function, &bitcode, true, linecoverage);
        } else if (configuration.includeNotCovered) {
          functionsUnderTest.emplace_back(&function, &bitcode);
        }
      }
    } else {
      if (configuration.includeNotCovered) {
        diagnostics.warning("-include-not-covered is enabled, but there is no coverage info!");
      }
      for (llvm::Function &function : module) {
        functionsUnderTest.emplace_back(&function, &bitcode, true);
      }
    }
  });

  MutatorsFactory mutatorsFactory(diagnostics);
  MutationsFinder mutationsFinder(mutatorsFactory.mutators(configuration.mutators), configuration);

  std::vector<InstructionSelectionTask> instructionSelectionTasks;
  instructionSelectionTasks.reserve(configuration.parallelization.workers);
  for (int i = 0; i < configuration.parallelization.workers; i++) {
    instructionSelectionTasks.emplace_back(filters.instructionFilters);
  }

  std::vector<int> Nothing;
  TaskExecutor<InstructionSelectionTask> selectionRunner(diagnostics,
                                                         "Instruction selection",
                                                         functionsUnderTest,
                                                         Nothing,
                                                         std::move(instructionSelectionTasks));
  selectionRunner.execute();

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, functionsUnderTest);
  std::vector<MutationPoint *> mutations = std::move(mutationPoints);

  for (auto filter : filters.mutationFilters) {
    std::vector<MutationFilterTask> tasks;
    tasks.reserve(configuration.parallelization.workers);
    for (int i = 0; i < configuration.parallelization.workers; i++) {
      tasks.emplace_back(*filter);
    }

    std::string label = std::string("Applying filter: ") + filter->name();
    std::vector<MutationPoint *> tmp;
    TaskExecutor<MutationFilterTask> filterRunner(
        diagnostics, label, mutations, tmp, std::move(tasks));
    filterRunner.execute();
    mutations = std::move(tmp);
  }

  singleTask.execute("Prepare mutations", [&]() {
    for (auto point : mutations) {
      point->getBitcode()->addMutation(point);
    }
  });

  singleTask.execute("Cloning functions for mutation",
                     [&]() { CloneMutatedFunctionsTask::cloneFunctions(bitcode); });

  singleTask.execute("Removing original functions",
                     [&]() { DeleteOriginalFunctionsTask::deleteFunctions(bitcode); });

  singleTask.execute("Redirect mutated functions",
                     [&]() { InsertMutationTrampolinesTask::insertTrampolines(bitcode); });

  TaskExecutor<ApplyMutationTask> applyMutations(
      diagnostics, "Applying mutations", mutations, Nothing, { ApplyMutationTask() });
  applyMutations.execute();
}

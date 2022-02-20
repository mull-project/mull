#include "mull/Driver.h"

#include "mull/BitcodeMetadataReader.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/BlockAddressFunctionFilter.h"
#include "mull/Filters/Filters.h"
#include "mull/Filters/JunkMutationFilter.h"
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

#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>

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
      mutants.push_back(
          std::make_unique<Mutant>(identifier, mutatorIdentifier, sourceLocation, endLocation));
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
          config.executable, {}, {}, config.timeout, config.captureTestOutput, false, std::nullopt);
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
      std::vector<InsertMutationTrampolinesTask>(workers, InsertMutationTrampolinesTask{ config }));
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
  Runner runner(diagnostics);
  MutantRunner mutantRunner(diagnostics, config, runner);
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

std::vector<FunctionUnderTest> Driver::getFunctionsUnderTest() {
  std::vector<FunctionUnderTest> functionsUnderTest;

  singleTask.execute("Gathering functions under test", [&]() {
    for (auto &bitcode : program.bitcode()) {
      for (llvm::Function &function : *bitcode->getModule()) {
        functionsUnderTest.emplace_back(&function, bitcode.get());
      }
    }
  });

  return functionsUnderTest;
}

static void printIR(llvm::Module &module, Diagnostics &diagnostics, bool toFile,
                    const std::string &suffix) {
  if (toFile) {
    std::error_code ec;
    auto filename = module.getSourceFileName() + suffix;
    llvm::raw_fd_ostream out(filename, ec);
    if (ec) {
      diagnostics.warning("Cannot create file for debugging: "s + filename + ": " + ec.message());
    }
    module.print(out, nullptr);
  } else {
    module.print(llvm::errs(), nullptr);
  }
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
  mull::Filters filters(configuration, diagnostics);
  filters.enableNoDebugFilter();
  filters.enableFilePathFilter();
  filters.enableGitDiffFilter();
  filters.enableBlockAddressFilter();
  filters.enableVariadicFunctionFilter();

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

  if (configuration.debug.printIR || configuration.debug.printIRBefore) {
    printIR(module, diagnostics, configuration.debug.printIRToFile, ".before.ll");
  }

  {
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    if (llvm::verifyModule(module, &errorStream)) {
      std::stringstream message;
      message << "Mull encountered broken LLVM module.\n"
              << "Please, report the following error message here "
                 "https://github.com/mull-project/mull/issues\n"
              << "Underlying error message:\n"
              << errorStream.str();
      diagnostics.error(message.str());
    }
  }

  Bitcode bitcode(&module);
  std::vector<FunctionUnderTest> functionsUnderTest;
  singleTask.execute("Gathering functions under test", [&]() {
    for (llvm::Function &function : module) {
      functionsUnderTest.emplace_back(&function, &bitcode);
    }
  });

  std::vector<FunctionUnderTest> filteredFunctions;
  for (const auto &function : functionsUnderTest) {
    bool skip = false;
    for (auto filter : filters.functionFilters) {
      if (filter->shouldSkip(function.getFunction())) {
        skip = true;
        break;
      }
    }
    if (!skip) {
      filteredFunctions.emplace_back(function);
    }
  }

  MutatorsFactory mutatorsFactory(diagnostics);
  MutationsFinder mutationsFinder(
      mutatorsFactory.mutators(configuration.mutators, configuration.ignoreMutators),
      configuration);

  std::vector<InstructionSelectionTask> instructionSelectionTasks;
  instructionSelectionTasks.reserve(configuration.parallelization.workers);
  for (int i = 0; i < configuration.parallelization.workers; i++) {
    instructionSelectionTasks.emplace_back(filters.instructionFilters);
  }

  std::vector<int> Nothing;
  TaskExecutor<InstructionSelectionTask> selectionRunner(diagnostics,
                                                         "Instruction selection",
                                                         filteredFunctions,
                                                         Nothing,
                                                         std::move(instructionSelectionTasks));
  selectionRunner.execute();

  std::vector<MutationPoint *> mutationPoints =
      mutationsFinder.getMutationPoints(diagnostics, filteredFunctions);
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

  singleTask.execute("Redirect mutated functions", [&]() {
    InsertMutationTrampolinesTask::insertTrampolines(bitcode, configuration);
  });

  TaskExecutor<ApplyMutationTask> applyMutations(
      diagnostics, "Applying mutations", mutations, Nothing, { ApplyMutationTask() });
  applyMutations.execute();

  if (configuration.debug.printIR || configuration.debug.printIRAfter) {
    printIR(module, diagnostics, configuration.debug.printIRToFile, ".after.ll");
  }

  {
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    if (llvm::verifyModule(module, &errorStream)) {
      std::stringstream message;
      message << "Uh oh! Mull corrupted LLVM module.\n"
              << "Please, report the following error message here "
                 "https://github.com/mull-project/mull/issues\n"
              << "Underlying error message:\n"
              << errorStream.str();
      diagnostics.error(message.str());
    }
  }
}

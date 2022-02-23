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
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"

#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std;

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

#include "mull/Driver.h"

#include "mull/BitcodeMetadataReader.h"
#include "mull/Filters/BlockAddressFunctionFilter.h"
#include "mull/Filters/Filters.h"
#include "mull/Filters/MutationPointFilter.h"
#include "mull/FunctionUnderTest.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Parallelization/Parallelization.h"
#include "mull/Program/Program.h"

#include "rust/mull-core/core.rs.h"

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
namespace {
void printIR(llvm::Module &module, const MullDiagnostics &diagnostics, bool toFile,
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

std::vector<MutationPoint *>
filterOutJunk(const MullDiagnostics &diagnostics, const MullConfig &configuration,
              std::vector<MutationPoint *> points,
              std::unordered_map<std::string, std::string> bitcodeCompilationFlags) {
  std::string cxxCompilationFlags;
  for (auto &flag : configuration.compiler_flags) {
    cxxCompilationFlags += std::string(flag) + ' ';
  }

  mull::ASTStorage astStorage(diagnostics,
                              std::string(configuration.compilation_database_path),
                              cxxCompilationFlags,
                              bitcodeCompilationFlags);

  mull::CXXJunkDetector junkDetector(diagnostics, astStorage);

  std::vector<MutationPoint *> out;
  SingleTaskExecutor singleTask(diagnostics);
  singleTask.execute("Applying filter: junk", [&]() {
    for (auto point : points) {
      if (!junkDetector.isJunk(point))
        out.emplace_back(point);
    }
  });

  return out;
}

std::vector<std::string> toStdVector(const rust::Vec<rust::String> &v) {
  std::vector<std::string> out;
  out.reserve(v.size());
  for (const auto &s : v) {
    out.emplace_back(std::string(s));
  }
  return out;
}

} // namespace

void mull::mutateBitcode(llvm::Module &module) {
  /// Setup
  auto core = init_core_ffi();
  const MullDiagnostics &diagnostics = core->diag();
  const MullConfig &configuration = core->config();

  std::vector<std::unique_ptr<mull::Filter>> filterStorage;
  mull::Filters filters(configuration, diagnostics);
  filters.enableNoDebugFilter();
  filters.enableFilePathFilter();
  filters.enableGitDiffFilter();
  filters.enableBlockAddressFilter();
  filters.enableVariadicFunctionFilter();
  filters.enableManualFilter();

  SingleTaskExecutor singleTask(diagnostics);

  /// Actual Work

  mull::BitcodeMetadataReader bitcodeCompilationDatabaseLoader;
  std::unordered_map<std::string, std::string> bitcodeCompilationFlags =
      bitcodeCompilationDatabaseLoader.getCompilationDatabase(module);
  if (!bitcodeCompilationFlags.empty()) {
    diagnostics.info(std::string("Found compilation flags in the input bitcode"));
  }

  /// How can I check that -g flag (debug info enable) was set, from llvm pass
  /// https://stackoverflow.com/a/21713717/598057
  if (module.getNamedMetadata("llvm.dbg.cu") == nullptr) {
    diagnostics.warning("Mull cannot find debug information. Recompile with `-g` flag.");
  }

  if (bitcodeCompilationFlags.empty()) {
    diagnostics.warning(
        "Mull cannot find compiler flags. Recompile with `-grecord-command-line` flag.");
  }

  if (configuration.debug.print_ir || configuration.debug.print_ir_before) {
    printIR(module, diagnostics, configuration.debug.print_ir_to_file, ".before.ll");
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
      mutatorsFactory.mutators(toStdVector(configuration.mutators),
                               toStdVector(configuration.ignore_mutators)),
      configuration);

  std::vector<InstructionSelectionTask> instructionSelectionTasks;
  instructionSelectionTasks.reserve(configuration.workers);
  for (unsigned i = 0; i < configuration.workers; i++) {
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
    tasks.reserve(configuration.workers);
    for (unsigned i = 0; i < configuration.workers; i++) {
      tasks.emplace_back(*filter);
    }

    std::string label = std::string("Applying filter: ") + filter->name();
    std::vector<MutationPoint *> tmp;
    TaskExecutor<MutationFilterTask> filterRunner(
        diagnostics, label, mutations, tmp, std::move(tasks));
    filterRunner.execute();
    mutations = std::move(tmp);
  }

  if (!configuration.junk_detection_disabled) {
    mutations =
        filterOutJunk(diagnostics, configuration, std::move(mutations), bitcodeCompilationFlags);
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

  TaskExecutor<ApplyMutationTask> applyMutations(diagnostics,
                                                 "Applying mutations",
                                                 mutations,
                                                 Nothing,
                                                 { ApplyMutationTask(configuration, diagnostics) });
  applyMutations.execute();

  if (configuration.debug.print_ir || configuration.debug.print_ir_after) {
    printIR(module, diagnostics, configuration.debug.print_ir_to_file, ".after.ll");
  }

  {
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    if (llvm::verifyModule(module, &errorStream)) {
      std::stringstream message;
      message << "Uh oh! Mull corrupted LLVM module.\n"
              << "Please, add the following lines:\n"
              << "debug:\n"
              << " slowIRVerification: true\n"
              << "to the config file ";
      if (!configuration.config_path.empty()) {
        message << "(" << std::string(configuration.config_path) << ") ";
      }
      message << "and re-run the failing command.\n\n";
      message << "Otherwise, report the following error message here "
                 "https://github.com/mull-project/mull/issues\n"
              << "Underlying error message:\n"
              << errorStream.str();
      diagnostics.error(message.str());
    }
  }
}

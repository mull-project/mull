#include "DynamicLibraries.h"
#include "MergeInstProfile.h"
#include "MutantExtractor.h"
#include "mull/Filters/CoverageFilter.h"
#include "mull/Filters/Filters.h"
#include "mull/Metrics/MetricsMeasure.h"
#include "mull/MutantRunner.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Result.h"
#include "mull/Runner.h"
#include "mull/Version.h"

#include "tools/CLIOptions/CLIOptions.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>

#include <algorithm>
#include <limits.h>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace std::string_literals;

static std::string validateInputFile(const std::string &inputFile,
                                     const MullDiagnostics &diagnostics) {
  if (access(inputFile.c_str(), R_OK) != 0) {
    diagnostics.error("The provided path to an executable program is not valid: "s + inputFile);
    return "";
  }
  llvm::SmallString<256> realPath;
  if (llvm::sys::fs::real_path(inputFile, realPath, false)) {
    diagnostics.error("The provided path to an executable program is not valid: "s + inputFile);
    return "";
  }
  return realPath.str().str();
}

int main(int argc, char **argv) {
  llvm::llvm_shutdown_obj llvmShutdownObj;

  for (int i = 1; i < argc; i++) {
    // TODO: Move elsewhere
    if (std::string(argv[i]) == "--dump-mutators" && i + 1 < argc) {
      auto core = init_core_ffi();
      tool::dumpMutators(core->diag(), argv[i + 1]);
      return 0;
    }
  }

  auto mullCLI =
      init_runner_cli(tool::argsFromArgv(argc, argv), rust::String(mull::llvmVersionString()));
  const auto &diagnostics = mullCLI->diag_cli();
  const auto &cli = mullCLI->cli();
  const auto &config = mullCLI->config_cli();

  std::string inputFile = validateInputFile(std::string(cli.input_file), diagnostics);

  mull::MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  auto params = tool::buildReporterParameters(cli,
                                              { { "URL", mull::mullHomepageString() },
                                                { "Mull Version", mull::mullVersionString() },
                                                { "LLVM Version", mull::llvmVersionString() } });
  auto reporters = tool::createReporters(diagnostics, params);

  std::string testProgram = inputFile;
  if (!std::string(cli.test_program).empty()) {
    testProgram = std::string(cli.test_program);
  }

  auto extraArgs = tool::toStdVector(cli.runner_args);
  auto librarySearchPaths = tool::toStdVector(cli.ld_search_paths);

  std::vector<std::string> mutantHolders({ inputFile });
  mull::resolveLibraries(diagnostics,
                         mutantHolders,
                         mull::getDynamicLibraryDependencies(diagnostics, inputFile),
                         librarySearchPaths);

  mull::Filters filters(config, diagnostics);
  filters.enableGitDiffFilter();
  filters.enableFilePathFilter();

  mull::MutantExtractor mutantExtractor(diagnostics);
  auto mutants = mutantExtractor.extractMutants(mutantHolders);
  diagnostics.debug("Found "s + std::to_string(mutants.size()) + " mutant(s)");
  std::string coverageInfo = std::string(cli.coverage_info);

  std::string rawCoverageData;
  std::unordered_map<std::string, std::string> env;
  if (coverageInfo.empty() && mull::hasCoverage(diagnostics, inputFile)) {
    llvm::SmallString<PATH_MAX> rawPath;
    llvm::sys::fs::getPotentiallyUniqueTempFileName("mull", "raw-coverage", rawPath);
    rawCoverageData = rawPath.str().str();

    llvm::SmallString<PATH_MAX> indexedPath;
    llvm::sys::fs::getPotentiallyUniqueTempFileName("mull", "indexed-coverage", indexedPath);
    coverageInfo = indexedPath.str().str();

    if (config.debug.coverage) {
      diagnostics.debug("rawCoverageFile: "s + rawCoverageData);
      diagnostics.debug("indexedCoverageFile: "s + coverageInfo);
    }

    env["LLVM_PROFILE_FILE"] = rawCoverageData;
  }

  mull::Runner runner(diagnostics);
  mull::SingleTaskExecutor singleTask(diagnostics);

  mull::ExecutionResult warmUpResult{};
  singleTask.execute("Warm up run", [&]() {
    warmUpResult = runner.runProgram(testProgram,
                                     extraArgs,
                                     env,
                                     config.timeout,
                                     config.capture_mutant_output,
                                     false,
                                     std::nullopt);
  });
  if (warmUpResult.status != mull::ExecutionStatus::Passed) {
    diagnostics.error(warmUpResult.debugDescription());
  }

  if (!rawCoverageData.empty()) {
    singleTask.execute("Extracting coverage information", [&]() {
      mull::mergeRawInstProfile(diagnostics, rawCoverageData, coverageInfo);
      llvm::sys::fs::remove(rawCoverageData);
    });
  }

  auto coverage = filters.enableCoverageFilter(coverageInfo, mutantHolders);
  for (auto &mutant : mutants) {
    if (coverage->covered(mutant.get())) {
      mutant->setCovered(true);
    }
  }

  std::vector<std::unique_ptr<mull::Mutant>> filteredMutants;
  singleTask.execute("Filter mutants", [&]() {
    for (auto &mutant : mutants) {
      bool skip = false;
      for (auto filter : filters.mutantFilters) {
        if (filter->shouldSkip(mutant.get())) {
          skip = true;
          if (config.debug.filters) {
            diagnostics.debug("Skipping "s + mutant->getIdentifier() + " due to " + filter->name() +
                              " filter");
          }
          break;
        }
      }
      if (!skip) {
        filteredMutants.push_back(std::move(mutant));
      }
    }
  });
  diagnostics.debug(std::to_string(filteredMutants.size()) + " mutant(s) after filter");

  mull::MutantRunner mutantRunner(diagnostics, config, runner);
  auto mutationResults = mutantRunner.runMutants(testProgram, extraArgs, filteredMutants);

  std::size_t surviving =
      std::count_if(std::cbegin(mutationResults), std::cend(mutationResults), [](auto const &r) {
        return r->getExecutionResult().status == mull::ExecutionStatus::Passed;
      });

  int killedMutantsCount = 0;
  int totalMutantsCount = 0;
  for (auto &mr : mutationResults) {
    totalMutantsCount++;
    auto status = mr->getExecutionResult().status;
    if (status != mull::ExecutionStatus::NotCovered && status != mull::ExecutionStatus::Passed) {
      killedMutantsCount++;
    }
  }
  auto score = int(double(killedMutantsCount) / double(totalMutantsCount) * 100);

  auto mullResult =
      std::make_unique<mull::Result>(std::move(filteredMutants), std::move(mutationResults));
  for (auto &reporter : reporters) {
    reporter->reportResults(*mullResult);
  }

  totalExecutionTime.finish();
  std::stringstream ss;
  ss << "Total execution time: " << totalExecutionTime.duration()
     << mull::MetricsMeasure::precision();
  diagnostics.info(ss.str());

  if (surviving) {
    ss.str(""s);
    ss << "Surviving mutants: " << surviving;
    diagnostics.info(ss.str());

    if (!cli.allow_surviving && cli.mutation_score_threshold > static_cast<uint32_t>(score)) {
      return 1;
    }
  }

  return 0;
}

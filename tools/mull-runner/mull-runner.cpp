#include "CoverageChecker.h"
#include "DynamicLibraries.h"
#include "MergeInstProfile.h"
#include "MutantExtractor.h"
#include "mull-runner-cli.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Metrics/MetricsMeasure.h"
#include "mull/MutantRunner.h"
#include "mull/Parallelization/TaskExecutor.h"
#include "mull/Result.h"
#include "mull/Toolchain/Runner.h"
#include "mull/Version.h"

#include <llvm/Support/FileSystem.h>

#include <memory>
#include <unistd.h>

using namespace std::string_literals;

static std::string validateInputFile(const std::string &inputFile, mull::Diagnostics &diagnostics) {
  if (access(inputFile.c_str(), R_OK) != 0) {
    diagnostics.error("The provided path to an executable program is not valid: "s + inputFile);
    return "";
  }
  llvm::SmallString<256> inputRealPath;
  if (llvm::sys::fs::real_path(inputFile, inputRealPath, false)) {
    diagnostics.error("The provided path to an executable program is not valid: "s + inputFile);
    return "";
  }
  return inputRealPath.str().str();
}

int main(int argc, char **argv) {
  llvm::llvm_shutdown_obj llvmShutdownObj;
  mull::Diagnostics diagnostics;
  llvm::cl::SetVersionPrinter(mull::printVersionInformation);

  tool::ReportersCLIOptions reportersOption(diagnostics, tool::ReportersOption);

  llvm::cl::HideUnrelatedOptions(tool::MullCategory);
  bool validOptions = llvm::cl::ParseCommandLineOptions(argc, argv, "", &llvm::errs());
  if (!validOptions) {
    if (tool::DumpCLIInterface) {
      tool::dumpCLIInterface(diagnostics);
      return 0;
    }
    return 1;
  }

  if (tool::DebugEnabled) {
    diagnostics.enableDebugMode();
    diagnostics.debug("Diagnostics: Debug Mode enabled. Debug-level messages will be printed.");
  }

  if (tool::StrictModeEnabled) {
    diagnostics.enableStrictMode();
    diagnostics.info(
        "Diagnostics: Strict Mode enabled. Warning messages will be treated as fatal errors.");
  }

  std::string inputFile = validateInputFile(tool::InputFile.getValue(), diagnostics);

  mull::MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  mull::Configuration configuration;
  auto configPath = mull::Configuration::findConfig(diagnostics);
  if (!configPath.empty()) {
    configuration = mull::Configuration::loadFromDisk(diagnostics, configPath);
    diagnostics.info("Using config "s + configPath);
  }

  if (tool::DebugEnabled.getNumOccurrences()) {
    configuration.debugEnabled = tool::DebugEnabled;
  }
  if (tool::Timeout.getNumOccurrences()) {
    configuration.timeout = tool::Timeout.getValue();
  }

  if (tool::IncludeNotCovered.getNumOccurrences()) {
    configuration.includeNotCovered = tool::IncludeNotCovered.getValue();
  }

  configuration.executable = inputFile;

  if (tool::Workers.getNumOccurrences()) {
    mull::ParallelizationConfig parallelizationConfig;
    parallelizationConfig.workers = tool::Workers;
    parallelizationConfig.normalize();
    if (parallelizationConfig.exceedsHardware()) {
      diagnostics.warning("You choose a number of workers that exceeds your number of cores. This "
                          "may lead to timeouts and incorrect results");
    }
    configuration.parallelization = parallelizationConfig;
  } else {
    configuration.parallelization = mull::ParallelizationConfig::defaultConfig();
  }

  if (tool::NoTestOutput.getNumOccurrences() || tool::NoOutput.getNumOccurrences()) {
    configuration.captureTestOutput = false;
  }
  if (tool::NoMutantOutput.getNumOccurrences() || tool::NoOutput.getNumOccurrences()) {
    configuration.captureMutantOutput = false;
  }

  configuration.debug.coverage = tool::DebugCoverage.getValue();

  tool::ReporterParameters params{ .reporterName = tool::ReportName.getValue(),
                                   .reporterDirectory = tool::ReportDirectory.getValue(),
                                   .patchBasePathDir = tool::ReportPatchBaseDirectory.getValue(),
                                   // we should not need the database at this point
                                   .compilationDatabaseAvailable = true,
                                   .IDEReporterShowKilled = tool::IDEReporterShowKilled,
                                   .mullInformation = std::unordered_map<std::string, std::string>{
                                       std::make_pair("URL", mull::mullHomepageString()),
                                       std::make_pair("Mull Version", mull::mullVersionString()),
                                       std::make_pair("Commit", mull::mullCommitString()),
                                       std::make_pair("Build Date", mull::mullBuildDateString()),
                                       std::make_pair("LLVM Version", mull::llvmVersionString()),
                                   } };
  std::vector<std::unique_ptr<mull::Reporter>> reporters = reportersOption.reporters(params);

  std::string testProgram = configuration.executable;
  if (!tool::TestProgram.empty()) {
    testProgram = tool::TestProgram.getValue();
  }

  std::vector<std::string> extraArgs;
  for (size_t argIndex = 0; argIndex < tool::RunnerArgs.getNumOccurrences(); argIndex++) {
    extraArgs.push_back(tool::RunnerArgs[argIndex]);
  }

  std::vector<std::string> librarySearchPaths(std::begin(tool::LDSearchPaths),
                                              std::end(tool::LDSearchPaths));

  std::vector<std::string> mutantHolders({ configuration.executable });
  // resolve any dynamic libraries listed as DT_NEEDED
  mull::resolveLibraries(diagnostics,
                         mutantHolders,
                         mull::getDynamicLibraryDependencies(diagnostics, configuration.executable),
                         librarySearchPaths);

  mull::MutantExtractor mutantExtractor(diagnostics);
  std::vector<std::unique_ptr<mull::Mutant>> mutants =
      mutantExtractor.extractMutants(mutantHolders);
  std::string coverageInfo = tool::CoverageInfo.getValue();

  std::string rawCoverageData;

  std::unordered_map<std::string, std::string> env;
  if (coverageInfo.empty() && mull::hasCoverage(diagnostics, configuration.executable)) {
    llvm::SmallString<PATH_MAX> rawPath;
    llvm::sys::fs::getPotentiallyUniqueTempFileName("mull", "raw-coverage", rawPath);
    rawCoverageData = rawPath.str().str();

    llvm::SmallString<PATH_MAX> indexedPath;
    llvm::sys::fs::getPotentiallyUniqueTempFileName("mull", "indexed-coverage", indexedPath);
    coverageInfo = indexedPath.str().str();

    if (configuration.debug.coverage) {
      llvm::errs() << "rawCoverageFile: " << rawCoverageData << "\n";
      llvm::errs() << "indexedCoverageFile: " << coverageInfo << "\n";
    }

    env["LLVM_PROFILE_FILE"] = rawCoverageData;
  }
  mull::Runner runner(diagnostics);
  mull::SingleTaskExecutor singleTask(diagnostics);
  /// On macOS, sometimes newly compiled programs take more time to execute for the first run
  /// As we take the execution time as a baseline for timeout it makes sense to have an additional
  /// warm up run so that the next runs will be a bit faster
  mull::ExecutionResult warmUpResult{};
  singleTask.execute("Warm up run", [&]() {
    warmUpResult = runner.runProgram(testProgram,
                                     extraArgs,
                                     env,
                                     configuration.timeout,
                                     configuration.captureMutantOutput,
                                     std::nullopt);
  });
  if (warmUpResult.status != mull::ExecutionStatus::Passed) {
    diagnostics.warning(warmUpResult.debugDescription());
  }

  if (!rawCoverageData.empty()) {
    singleTask.execute("Extracting coverage information", [&]() {
      mull::mergeRawInstProfile(diagnostics, rawCoverageData, coverageInfo);
      llvm::sys::fs::remove(rawCoverageData);
    });
  }

  std::vector<std::unique_ptr<mull::Mutant>> filteredMutants;
  mull::CoverageChecker coverage(configuration, diagnostics, coverageInfo, mutantHolders);
  for (auto &mutant : mutants) {
    bool covered = coverage.covered(mutant.get());
    mutant->setCovered(covered);
    if (covered || configuration.includeNotCovered) {
      filteredMutants.push_back(std::move(mutant));
    }
  }

  mull::MutantRunner mutantRunner(diagnostics, configuration, runner);
  std::vector<std::unique_ptr<mull::MutationResult>> mutationResults =
      mutantRunner.runMutants(testProgram, extraArgs, filteredMutants);

  auto result =
      std::make_unique<mull::Result>(std::move(filteredMutants), std::move(mutationResults));
  for (auto &reporter : reporters) {
    reporter->reportResults(*result);
  }

  totalExecutionTime.finish();
  std::stringstream stringstream;
  stringstream << "Total execution time: " << totalExecutionTime.duration()
               << mull::MetricsMeasure::precision();
  diagnostics.info(stringstream.str());

  return 0;
}

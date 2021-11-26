#include "MutantExtractor.h"
#include "mull-runner-cli.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Metrics/MetricsMeasure.h"
#include "mull/MutantRunner.h"
#include "mull/Result.h"
#include "mull/Version.h"
#include <llvm/Support/FileSystem.h>

#include <memory>
#include <unistd.h>

static std::string validateInputFile(const std::string &inputFile, mull::Diagnostics &diagnostics) {
  if (access(inputFile.c_str(), R_OK) != 0) {
    diagnostics.error(std::string("The provided path to an executable program is not valid: ") +
                      inputFile.c_str());
    return "";
  }
  llvm::SmallString<256> inputRealPath;
  if (llvm::sys::fs::real_path(inputFile, inputRealPath, false)) {
    diagnostics.error(std::string("The provided path to an executable program is not valid: ") +
                      inputFile.c_str());
    return "";
  }
  return inputRealPath.str().str();
}

int main(int argc, char **argv) {
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

  configuration.debugEnabled = tool::DebugEnabled;
  configuration.timeout = tool::Timeout.getValue();
  configuration.includeNotCovered = tool::IncludeNotCovered.getValue();

  configuration.executable = inputFile;

  if (tool::Workers) {
    mull::ParallelizationConfig parallelizationConfig;
    parallelizationConfig.workers = tool::Workers;
    parallelizationConfig.normalize();
    if (parallelizationConfig.exceedsHardware()){
      diagnostics.warning("You choose a number of workers that exceeds your number of cores. This may lead to timeouts and incorrect results");
    }
    configuration.parallelization = parallelizationConfig;
  } else {
    configuration.parallelization = mull::ParallelizationConfig::defaultConfig();
  }

  if (tool::NoTestOutput.getValue() || tool::NoOutput.getValue()) {
    configuration.captureTestOutput = false;
  }
  if (tool::NoMutantOutput.getValue() || tool::NoOutput.getValue()) {
    configuration.captureMutantOutput = false;
  }

  tool::ReporterParameters params{ .reporterName = tool::ReportName.getValue(),
                                   .reporterDirectory = tool::ReportDirectory.getValue(),
                                   .patchBasePathDir = tool::ReportPatchBaseDirectory.getValue(),
                                   .compilationDatabaseAvailable = false,
                                   .IDEReporterShowKilled = tool::IDEReporterShowKilled };
  std::vector<std::unique_ptr<mull::Reporter>> reporters = reportersOption.reporters(params);

  std::string executable = inputFile;
  std::string testProgram = executable;
  if (!tool::TestProgram.empty()) {
    testProgram = tool::TestProgram.getValue();
  }

  std::vector<std::string> extraArgs;
  for (size_t argIndex = 0; argIndex < tool::RunnerArgs.getNumOccurrences(); argIndex++) {
    extraArgs.push_back(tool::RunnerArgs[argIndex]);
  }

  mull::MutantExtractor mutantExtractor(diagnostics);
  std::vector<std::unique_ptr<mull::Mutant>> mutants = mutantExtractor.extractMutants(executable);

  mull::MutantRunner mutantRunner(diagnostics, configuration);
  std::vector<std::unique_ptr<mull::MutationResult>> mutationResults =
      mutantRunner.runMutants(testProgram, extraArgs, mutants);

  auto result = std::make_unique<mull::Result>(std::move(mutants), std::move(mutationResults));
  for (auto &reporter : reporters) {
    reporter->reportResults(*result);
  }
  llvm::llvm_shutdown();
  totalExecutionTime.finish();
  std::stringstream stringstream;
  stringstream << "Total execution time: " << totalExecutionTime.duration()
               << mull::MetricsMeasure::precision();
  diagnostics.info(stringstream.str());

  return 0;
}

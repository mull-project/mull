#include "MutantExtractor.h"
#include "mull-runner-cli.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Metrics/MetricsMeasure.h"
#include "mull/MutantRunner.h"
#include "mull/Reporters/ASTSourceInfoProvider.h"
#include "mull/Result.h"
#include "mull/Version.h"

#include <memory>
#include <unistd.h>

static void validateInputFile() {
  if (access(tool::InputFile.getValue().c_str(), R_OK) != 0) {
    perror(tool::InputFile.getValue().c_str());
    exit(1);
  }
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

  validateInputFile();

  mull::MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  mull::Configuration configuration;

  configuration.debugEnabled = tool::DebugEnabled;
  configuration.timeout = tool::Timeout.getValue();
  configuration.includeNotCovered = tool::IncludeNotCovered.getValue();

  configuration.executable = tool::InputFile.getValue();

  if (tool::Workers) {
    mull::ParallelizationConfig parallelizationConfig;
    parallelizationConfig.workers = tool::Workers;
    parallelizationConfig.normalize();
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

  mull::NullSourceInfoProvider sourceInfoProvider;
  tool::ReporterParameters params{ .reporterName = tool::ReportName.getValue(),
                                   .reporterDirectory = tool::ReportDirectory.getValue(),
                                   .sourceInfoProvider = sourceInfoProvider,
                                   .compilationDatabaseAvailable = false,
                                   .IDEReporterShowKilled = tool::IDEReporterShowKilled };
  std::vector<std::unique_ptr<mull::Reporter>> reporters = reportersOption.reporters(params);

  std::string executable = tool::InputFile.getValue();
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

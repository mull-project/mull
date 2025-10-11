#include "mull-reporter-cli.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Metrics/MetricsMeasure.h"
#include "mull/Reporters/SQLiteReporter.h"
#include "mull/Result.h"
#include "mull/Version.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>

#include <algorithm>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <unistd.h>
#include <utility>

using namespace std::string_literals;

static std::string validateInputFile(const std::string &inputFile, mull::Diagnostics &diagnostics) {
  if (access(inputFile.c_str(), R_OK) != 0) {
    diagnostics.error("Cannot access sqlite report: "s + inputFile);
    return "";
  }
  llvm::SmallString<256> inputRealPath;
  if (llvm::sys::fs::real_path(inputFile, inputRealPath, false)) {
    diagnostics.error("Cannot access sqlite report: "s + inputFile);
    return "";
  }
  return inputRealPath.str().str();
}

static std::vector<std::string> split(const std::string &input, char delimiter) {
  std::vector<std::string> output;
  std::string string;
  std::istringstream stream(input);
  while (std::getline(stream, string, delimiter)) {
    if (!string.empty()) {
      output.push_back(string);
    }
  }
  return output;
}

static std::unique_ptr<mull::Mutant> mutantFromId(const std::string &mutantId) {
  std::vector<std::string> chunks = split(mutantId, ':');

  std::string mutator = chunks[0];
  std::string location = chunks[1];
  int beginLine = std::stoi(chunks[2]);
  int beginColumn = std::stoi(chunks[3]);
  int endLine = std::stoi(chunks[4]);
  int endColumn = std::stoi(chunks[5]);

  return std::make_unique<mull::Mutant>(
      mutantId,
      mutator,
      mull::SourceLocation("", location, "", location, beginLine, beginColumn),
      mull::SourceLocation("", location, "", location, endLine, endColumn));
}

static mull::ExecutionResult
chooseExecutionResult(const std::vector<mull::ExecutionResult> &results) {
  if (results.size() == 1) {
    return results[0];
  }

#define SELECT_MATCHING_STATUS(STATUS)                                                             \
  for (auto &result : results) {                                                                   \
    if (result.status == mull::ExecutionStatus::STATUS) {                                          \
      return result;                                                                               \
    }                                                                                              \
  }
  // Among several execution results for each mutant, we need to only pick
  // Various execution statuses are ordered in a somewhat arbitrary way
  // 'Passed' always has the lowest priority

  // TODO: That's kinda quadratic, perhaps there is a way to speed this up?
  SELECT_MATCHING_STATUS(Failed);
  SELECT_MATCHING_STATUS(Crashed);
  SELECT_MATCHING_STATUS(Timedout);
  SELECT_MATCHING_STATUS(AbnormalExit);
  SELECT_MATCHING_STATUS(FailFast);
  SELECT_MATCHING_STATUS(NotCovered);
  SELECT_MATCHING_STATUS(DryRun);
  SELECT_MATCHING_STATUS(Passed);

  return results[0];
}

int main(int argc, char **argv) {
  llvm::llvm_shutdown_obj llvmShutdownObj;
  mull::Diagnostics diagnostics;
  llvm::cl::SetVersionPrinter(mull::printVersionInformation);

  tool::ReportersCLIOptions reportersOption(diagnostics, tool::ReportersOption);

  llvm::cl::HideUnrelatedOptions(tool::MullCategory);
  bool validOptions = llvm::cl::ParseCommandLineOptions(argc, argv, "", &llvm::errs());
  if (!validOptions) {
    if (!tool::DumpCLIInterface.getValue().empty()) {
      tool::dumpCLIInterface(diagnostics, tool::DumpCLIInterface.getValue());
      return 0;
    }
    return 1;
  }

  std::string inputFile = validateInputFile(tool::SQLiteReport.getValue(), diagnostics);

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
  if (configuration.debugEnabled) {
    diagnostics.enableDebugMode();
    diagnostics.debug("Diagnostics: Debug Mode enabled. Debug-level messages will be printed.");
  }

  if (tool::StrictModeEnabled) {
    diagnostics.enableStrictMode();
    diagnostics.info(
        "Diagnostics: Strict Mode enabled. Warning messages will be treated as fatal errors.");
  }

  if (tool::NoTestOutput.getNumOccurrences() || tool::NoOutput.getNumOccurrences()) {
    configuration.captureTestOutput = false;
  }
  if (tool::NoMutantOutput.getNumOccurrences() || tool::NoOutput.getNumOccurrences()) {
    configuration.captureMutantOutput = false;
  }

  std::vector<std::unique_ptr<mull::Mutant>> mutants;
  std::vector<std::unique_ptr<mull::MutationResult>> mutationResults;
  auto raw = mull::SQLiteReporter::loadRawReport(tool::SQLiteReport.getValue());
  for (auto &[mutandId, executionResults] : raw.executionResults) {
    auto mutant = mutantFromId(mutandId);
    auto executionResult = chooseExecutionResult(executionResults);
    mutants.push_back(std::move(mutant));
    mutationResults.push_back(
        std::make_unique<mull::MutationResult>(executionResult, mutants.back().get()));
  }
  std::sort(std::begin(mutants), std::end(mutants), mull::MutantComparator());
  std::sort(
      std::begin(mutationResults), std::end(mutationResults), mull::MutationResultComparator());

  tool::ReporterParameters params{ .reporterName = tool::ReportName.getValue(),
                                   .reporterDirectory = tool::ReportDirectory.getValue(),
                                   .patchBasePathDir = tool::ReportPatchBaseDirectory.getValue(),
                                   // we should not need the database at this point
                                   .compilationDatabaseAvailable = true,
                                   .IDEReporterShowKilled = tool::IDEReporterShowKilled,
                                   .mullInformation = raw.info };

  std::vector<std::unique_ptr<mull::Reporter>> reporters = reportersOption.reporters(params);

  // Count surviving mutants, for later
  std::size_t surviving = std::count_if(
      std::cbegin(mutationResults), std::cend(mutationResults), [](auto const &resPtr) {
        return resPtr->getExecutionResult().status == mull::ExecutionStatus::Passed;
      });

  // Calculate mutation score for later threshold comparison
  int killedMutantsCount = 0;
  int totalMutantsCount = 0;

  for (auto &mutationResult : mutationResults) {
    auto &executionResult = mutationResult->getExecutionResult();

    totalMutantsCount++;
    if ((executionResult.status != mull::ExecutionStatus::NotCovered) &&
        (executionResult.status != mull::ExecutionStatus::Passed)) {
      killedMutantsCount++;
    }
  }

  auto rawScore = double(killedMutantsCount) / double(totalMutantsCount);
  auto score = int(rawScore * 100);

  auto result = std::make_unique<mull::Result>(std::move(mutants), std::move(mutationResults));
  for (auto &reporter : reporters) {
    reporter->reportResults(*result);
  }

  totalExecutionTime.finish();
  std::stringstream stringstream;
  stringstream << "Total reporting time: " << totalExecutionTime.duration()
               << mull::MetricsMeasure::precision();
  diagnostics.info(stringstream.str());

  if (surviving) {
    stringstream.str(""s);
    stringstream << "Surviving mutants: " << surviving;
    diagnostics.info(stringstream.str());

    if ((!tool::AllowSurvivingEnabled) && (tool::MutationScoreThreshold > score)) {
      return 1;
    }
  }

  return 0;
}

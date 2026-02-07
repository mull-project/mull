#include "mull/Metrics/MetricsMeasure.h"
#include "mull/Reporters/SQLiteReporter.h"
#include "mull/Result.h"
#include "mull/Version.h"
#include "tools/CLIOptions/CLIOptions.h"

#include "rust/mull-core/core.rs.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>

#include <algorithm>
#include <memory>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace std::string_literals;

static std::string validateInputFile(const std::string &inputFile,
                                     const MullDiagnostics &diagnostics) {
  if (access(inputFile.c_str(), R_OK) != 0) {
    diagnostics.error("Cannot access sqlite report: "s + inputFile);
    return "";
  }
  llvm::SmallString<256> realPath;
  if (llvm::sys::fs::real_path(inputFile, realPath, false)) {
    diagnostics.error("Cannot access sqlite report: "s + inputFile);
    return "";
  }
  return realPath.str().str();
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
  auto mullCLI =
      init_reporter_cli(tool::argsFromArgv(argc, argv), rust::String(mull::llvmVersionString()));
  const auto &diagnostics = mullCLI->diag_cli();
  const auto &cli = mullCLI->cli();

  std::string inputFile = validateInputFile(std::string(cli.sqlite_report), diagnostics);

  mull::MetricsMeasure totalExecutionTime;
  totalExecutionTime.start();

  std::vector<std::unique_ptr<mull::Mutant>> mutants;
  std::vector<std::unique_ptr<mull::MutationResult>> mutationResults;
  auto raw = mull::SQLiteReporter::loadRawReport(std::string(cli.sqlite_report));
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

  auto params = tool::buildReporterParameters(cli, raw.info);
  auto reporters = tool::createReporters(diagnostics, params);

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

  auto mullResult = std::make_unique<mull::Result>(std::move(mutants), std::move(mutationResults));
  for (auto &reporter : reporters) {
    reporter->reportResults(*mullResult);
  }

  totalExecutionTime.finish();
  std::stringstream ss;
  ss << "Total reporting time: " << totalExecutionTime.duration()
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

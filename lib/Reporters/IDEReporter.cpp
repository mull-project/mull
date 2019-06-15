#include "mull/Reporters/IDEReporter.h"

#include "mull/Logger.h"
#include "mull/MutationResult.h"
#include "mull/Reporters/SourceManager.h"
#include "mull/Result.h"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace mull;

static bool mutantSurvived(const ExecutionStatus &status) {
  return status == ExecutionStatus::Passed;
}

static void printSurvivedMutant(SourceManager &sourceManager,
                                const MutationPoint &mutant) {
  auto &sourceLocation = mutant.getSourceLocation();
  assert(!sourceLocation.isNull() && "Debug information is missing?");
  Logger::info() << sourceLocation.filePath << ":" << sourceLocation.line << ":"
                 << sourceLocation.column
                 << ": warning: " << mutant.getDiagnostics() << "\n";
  auto line = sourceManager.getLine(sourceLocation);
  assert(sourceLocation.column < line.size());
  std::string caret(sourceLocation.column, ' ');
  for (size_t index = 0; index < sourceLocation.column; index++) {
    if (line[index] == '\t') {
      caret[index] = '\t';
    }
  }
  caret[sourceLocation.column - 1] = '^';

  Logger::info() << line;
  Logger::info() << caret << "\n";
}

void IDEReporter::reportResults(const Result &result, const RawConfig &config,
                                const Metrics &metrics) {
  if (result.getMutationPoints().empty()) {
    Logger::info() << "No mutants found. Mutation score: infinitely high\n";
    return;
  }

  std::set<MutationPoint *> killedMutants;
  for (auto &mutationResult : result.getMutationResults()) {
    auto mutant = mutationResult->getMutationPoint();
    auto &executionResult = mutationResult->getExecutionResult();

    if (!mutantSurvived(executionResult.status)) {
      killedMutants.insert(mutant);
    }
  }

  auto survivedMutantsCount =
      result.getMutationPoints().size() - killedMutants.size();

  Logger::info() << "\nSurvived mutants (" << survivedMutantsCount << "/"
                 << result.getMutationPoints().size() << "):\n\n";

  SourceManager sourceManager;
  for (auto mutant : result.getMutationPoints()) {
    if (killedMutants.find(mutant) == killedMutants.end()) {
      printSurvivedMutant(sourceManager, *mutant);
    }
  }

  auto rawScore =
      double(killedMutants.size()) / double(result.getMutationPoints().size());
  auto score = uint(rawScore * 100);
  Logger::info() << "Mutation score: " << score << "%\n";
}

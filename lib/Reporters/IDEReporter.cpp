#include "mull/Reporters/IDEReporter.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Reporters/SourceManager.h"
#include "mull/Result.h"

#include <assert.h>
#include <map>
#include <set>
#include <sstream>
#include <utility>

using namespace mull;

static bool mutantSurvived(const ExecutionStatus &status) {
  return status == ExecutionStatus::Passed;
}

static void printMutant(Diagnostics &diagnostics, SourceManager &sourceManager,
                        const MutationPoint &mutant, bool survived) {
  auto &sourceLocation = mutant.getSourceLocation();
  assert(!sourceLocation.isNull() && "Debug information is missing?");

  std::stringstream stringstream;
  stringstream << sourceLocation.filePath << ":" << sourceLocation.line << ":"
               << sourceLocation.column << ": warning: " << (survived ? "Survived" : "Killed")
               << ": " << mutant.getDiagnostics() << " [" << mutant.getMutatorIdentifier() << "]"
               << "\n";
  auto line = sourceManager.getLine(sourceLocation);
  assert(sourceLocation.column < line.size());

  std::string caret(sourceLocation.column, ' ');
  for (size_t index = 0; index < sourceLocation.column; index++) {
    if (line[index] == '\t') {
      caret[index] = '\t';
    }
  }
  caret[sourceLocation.column - 1] = '^';

  stringstream << line << caret << "\n";
  fprintf(stdout, "%s", stringstream.str().c_str());
  fflush(stdout);
}

IDEReporter::IDEReporter(Diagnostics &diagnostics, bool showKilled)
    : diagnostics(diagnostics), showKilled(showKilled) {}

void IDEReporter::reportResults(const Result &result, const Metrics &metrics) {
  if (result.getMutationPoints().empty()) {
    diagnostics.info("No mutants found. Mutation score: infinitely high");
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

  auto survivedMutantsCount = result.getMutationPoints().size() - killedMutants.size();
  auto killedMutantsCount = killedMutants.size();

  SourceManager sourceManager;

  /// It is important that below we iterate over result.getMutationPoints()
  /// because we want the output to be stable across multiple executions of Mull.
  /// Optimizing this here was a bad idea: https://github.com/mull-project/mull/pull/640.
  if (showKilled && killedMutantsCount > 0) {
    std::stringstream stringstream;
    stringstream << "Killed mutants (" << killedMutantsCount << "/"
                 << result.getMutationPoints().size() << "):";
    diagnostics.info(stringstream.str());
    for (auto &mutant : result.getMutationPoints()) {
      if (killedMutants.find(mutant) != killedMutants.end()) {
        printMutant(diagnostics, sourceManager, *mutant, false);
      }
    }
  }

  if (survivedMutantsCount > 0) {
    std::stringstream stringstream;
    stringstream << "Survived mutants (" << survivedMutantsCount << "/"
                 << result.getMutationPoints().size() << "):";
    diagnostics.info(stringstream.str());

    for (auto &mutant : result.getMutationPoints()) {
      if (killedMutants.find(mutant) == killedMutants.end()) {
        printMutant(diagnostics, sourceManager, *mutant, true);
      }
    }
  } else {
    diagnostics.info("All mutations have been killed");
  }

  auto rawScore = double(killedMutants.size()) / double(result.getMutationPoints().size());
  auto score = uint(rawScore * 100);
  diagnostics.info(std::string("Mutation score: ") + std::to_string(score) + '%');
}

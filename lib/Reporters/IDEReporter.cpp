#include "mull/Reporters/IDEReporter.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Mutant.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Reporters/SourceCodeReader.h"
#include "mull/Result.h"

#include <cassert>
#include <map>
#include <set>
#include <sstream>
#include <utility>

using namespace mull;
using namespace std::string_literals;

static bool mutantSurvived(const ExecutionStatus &status) {
  return status == ExecutionStatus::Passed;
}

static bool mutantNotCovered(const ExecutionStatus &status) {
  return status == ExecutionStatus::NotCovered;
}

static void printMutant(Diagnostics &diagnostics, MutatorsFactory &factory,
                        SourceCodeReader &sourceCodeReader, const Mutant &mutant,
                        const std::string &status) {
  auto &sourceLocation = mutant.getSourceLocation();
  if (sourceLocation.isNull() || !sourceLocation.canRead()) {
    diagnostics.warning("IDEReporter: Cannot report '"s + mutant.getIdentifier() +
                        "': cannot read "s + sourceLocation.filePath);
    return;
  }

  auto mutator = factory.getMutator(mutant.getMutatorIdentifier());
  std::stringstream stringstream;
  stringstream << sourceLocation.filePath << ":" << sourceLocation.line << ":"
               << sourceLocation.column << ": warning: " << status << ": "
               << mutator->getDiagnostics() << " [" << mutant.getMutatorIdentifier() << "]"
               << "\n";

  stringstream << sourceCodeReader.getSourceLineWithCaret(sourceLocation);

  fprintf(stdout, "%s", stringstream.str().c_str());
  fflush(stdout);
}

static void printMutants(Diagnostics &diagnostics, MutatorsFactory &factory,
                         SourceCodeReader &reader, const std::vector<Mutant *> &mutants,
                         size_t totalSize, const std::string &status) {
  if (mutants.empty()) {
    return;
  }
  std::stringstream stringstream;
  stringstream << status << " mutants (" << mutants.size() << "/" << totalSize << "):";
  diagnostics.info(stringstream.str());
  for (auto mutant : mutants) {
    printMutant(diagnostics, factory, reader, *mutant, status);
  }
}

IDEReporter::IDEReporter(Diagnostics &diagnostics, bool showKilled)
    : diagnostics(diagnostics), showKilled(showKilled), sourceCodeReader() {}

void IDEReporter::reportResults(const Result &result) {
  if (result.getMutants().empty()) {
    diagnostics.info("No mutants found. Mutation score: infinitely high");
    return;
  }

  std::vector<Mutant *> killedMutants;
  std::vector<Mutant *> survivedMutants;
  std::vector<Mutant *> notCoveredMutants;
  for (auto &mutationResult : result.getMutationResults()) {
    auto mutant = mutationResult->getMutant();
    auto &executionResult = mutationResult->getExecutionResult();

    if (mutantSurvived(executionResult.status)) {
      survivedMutants.push_back(mutant);
    } else if (mutantNotCovered(executionResult.status)) {
      notCoveredMutants.push_back(mutant);
    } else {
      killedMutants.push_back(mutant);
    }
  }

  assert(killedMutants.size() + survivedMutants.size() + notCoveredMutants.size() ==
         result.getMutationResults().size());

  MutatorsFactory factory(diagnostics);
  factory.init();

  if (showKilled) {
    printMutants(diagnostics,
                 factory,
                 sourceCodeReader,
                 killedMutants,
                 result.getMutants().size(),
                 "Killed");
  }

  printMutants(diagnostics,
               factory,
               sourceCodeReader,
               survivedMutants,
               result.getMutants().size(),
               "Survived");
  printMutants(diagnostics,
               factory,
               sourceCodeReader,
               notCoveredMutants,
               result.getMutants().size(),
               "Not Covered");

  if (survivedMutants.empty() && notCoveredMutants.empty()) {
    diagnostics.info("All mutations have been killed");
  }

  auto rawScore = double(killedMutants.size()) / double(result.getMutants().size());
  auto score = int(rawScore * 100);
  diagnostics.info(std::string("Mutation score: ") + std::to_string(score) + '%');
}

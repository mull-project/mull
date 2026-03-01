#include "mull/Reporters/IDEReporter.h"

#include "mull/Mutant.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Reporters/SourceCodeReader.h"
#include "mull/Result.h"

#include "rust/mull-core/core.rs.h"

#include <cassert>
#include <fstream>
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

static void printMutant(const MullDiagnostics &diagnostics, const std::string &reportFilePath,
                        MutatorsFactory &factory, SourceCodeReader &sourceCodeReader,
                        const Mutant &mutant, const std::string &status) {
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

  if (reportFilePath.empty()) {
    fprintf(stdout, "%s", stringstream.str().c_str());
    fflush(stdout);
  } else {
    std::ofstream s{ reportFilePath, std::ios::app };
    s << stringstream.str();
  }
}

static void printMutants(const MullDiagnostics &diagnostics, const std::string &reportFilePath,
                         MutatorsFactory &factory, SourceCodeReader &reader,
                         const std::vector<Mutant *> &mutants, size_t totalSize,
                         const std::string &status) {
  if (mutants.empty()) {
    return;
  }
  std::stringstream stringstream;
  stringstream << status << " mutants (" << mutants.size() << "/" << totalSize << "):";
  if (reportFilePath.empty()) {
    diagnostics.info(stringstream.str());
  } else {
    std::ofstream s{ reportFilePath, std::ios::app };
    s << stringstream.str() << "\n";
  }

  for (auto mutant : mutants) {
    printMutant(diagnostics, reportFilePath, factory, reader, *mutant, status);
  }
}

static std::string getReportDir(const std::string &reportDir) {
  if (reportDir.empty()) {
    return ".";
  }
  return reportDir;
}

IDEReporter::IDEReporter(const MullDiagnostics &diagnostics, bool showKilled,
                         const std::string &reportDir, const std::string &reportName)
    : diagnostics(diagnostics), showKilled(showKilled), sourceCodeReader(), reportFilePath() {
  if (!reportName.empty()) {
    reportFilePath = getReportDir(reportDir) + "/" + reportName + ".txt";
    std::fstream s{ reportFilePath, std::ios::trunc | std::ios::out };
  }
}

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
                 reportFilePath,
                 factory,
                 sourceCodeReader,
                 killedMutants,
                 result.getMutants().size(),
                 "Killed");
  }

  printMutants(diagnostics,
               reportFilePath,
               factory,
               sourceCodeReader,
               survivedMutants,
               result.getMutants().size(),
               "Survived");
  printMutants(diagnostics,
               reportFilePath,
               factory,
               sourceCodeReader,
               notCoveredMutants,
               result.getMutants().size(),
               "Not Covered");

  auto rawScore = double(killedMutants.size()) / double(result.getMutants().size());
  auto score = int(rawScore * 100);
  std::string scoreMsg = std::string("Mutation score: ") + std::to_string(score) + '%';

  if (reportFilePath.empty()) {
    if (survivedMutants.empty() && notCoveredMutants.empty()) {
      diagnostics.info("All mutations have been killed");
    }
    diagnostics.info(scoreMsg);
  } else {
    std::ofstream s{ reportFilePath, std::ios::app };

    if (survivedMutants.empty() && notCoveredMutants.empty()) {
      s << "All mutations have been killed\n";
    }
    s << scoreMsg << "\n";
  }
}

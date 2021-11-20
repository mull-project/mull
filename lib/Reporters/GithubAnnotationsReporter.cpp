#include "mull/Reporters/GithubAnnotationsReporter.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/ExecutionResult.h"
#include "mull/Mutant.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/Result.h"
#include "mull/SourceLocation.h"
#include "mull/Reporters/SourceCodeReader.h"

#include <sstream>
#include <string>

using namespace mull;


GithubAnnotationsReporter::GithubAnnotationsReporter(Diagnostics &diagnostics)
    : diagnostics(diagnostics){
}

void mull::GithubAnnotationsReporter::reportResults(const Result &result) {
  MutatorsFactory factory(diagnostics);
  factory.init();
  std::string level="warning";
  diagnostics.info("Github Annotations:");
  for (auto &mutationResult : result.getMutationResults()) {
    const ExecutionResult mutationExecutionResult = mutationResult->getExecutionResult();

    const auto mutant = *mutationResult->getMutant();
    const auto& sourceLocation = mutant.getSourceLocation();
    const auto& sourceEndLocation = mutant.getEndLocation();
    const auto mutator = factory.getMutator(mutant.getMutatorIdentifier());

    if (mutationExecutionResult.status != ExecutionStatus::Passed){
      continue;
    }

    std::stringstream stringstream;
    stringstream << "::" << level << " "
                 << "file=" << sourceLocation.filePath << ","
                 << "line=" << sourceLocation.line << ","
                 << "col=" << sourceLocation.column << ","
                 << "endLine=" << sourceEndLocation.line << ","
                 << "endColumn=" << sourceEndLocation.column
                 << "::"
                 << "[" << mutant.getMutatorIdentifier() << "] "
                 << mutator->getDiagnostics()
                 << "\n";

    fprintf(stdout, "%s", stringstream.str().c_str());
    fflush(stdout);
  }

}

#include "mull/Reporters/GithubAnnotationsReporter.h"

#include "mull/Core/ExecutionResult.h"
#include "mull/Core/Mutant.h"
#include "mull/Core/Result.h"
#include "mull/Core/SourceLocation.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Mutation/Bitcode.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Mutators/MutatorsFactory.h"
#include "mull/SourceManager/SourceCodeReader.h"

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

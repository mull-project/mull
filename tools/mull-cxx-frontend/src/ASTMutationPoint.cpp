#include "ASTMutationPoint.h"

#include <sstream>

ASTMutationPoint::ASTMutationPoint(std::unique_ptr<ASTMutation> mutation,
                                   mull::MutatorKind mutationType, std::string mutationIdentifier,
                                   clang::Stmt *toBeMutatedStmt, std::string sourceFilePath,
                                   int line, int column)
    : mutation(std::move(mutation)), mutationType(mutationType), mutableStmt(toBeMutatedStmt),
      sourceFilePath(sourceFilePath), line(line), column(column) {
  std::ostringstream mis;
  /// mutator:file:line:col:1
  mis << mutationIdentifier << ":" << sourceFilePath << ":" << line << ":" << column;
  this->mutationIdentifier = mis.str();
}

void ASTMutationPoint::performMutation(ASTMutator &mutator) {
  mutation->performMutation(*this, mutator);
}

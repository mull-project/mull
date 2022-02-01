#include "ASTMutationPoint.h"

#include <sstream>

namespace mull {
namespace cxx {

ASTMutationPoint::ASTMutationPoint(std::unique_ptr<ASTMutation> mutation,
                                   mull::MutatorKind mutationType, std::string mutationIdentifier,
                                   clang::Stmt *toBeMutatedStmt, std::string sourceFilePath,
                                   int beginLine, int beginColumn, int endLine, int endColumn)
    : mutation(std::move(mutation)), mutationType(mutationType), mutableStmt(toBeMutatedStmt),
      sourceFilePath(sourceFilePath), beginLine(beginLine), beginColumn(beginColumn),
      endLine(endLine), endColumn(endColumn) {
  std::ostringstream mis;
  /// mutator:file:line:col:1
  mis << mutationIdentifier << ":" << sourceFilePath << ":" << beginLine << ":" << beginColumn;
  this->mutationIdentifier = mis.str();

  std::ostringstream mbis;

  /// mutator:file:begin_line:begin_col:end_line:end_col
  mbis << mutationIdentifier << ":" << sourceFilePath << ":" << beginLine << ":" << beginColumn
       << ":" << endLine << ":" << endColumn;
  this->mutationBinaryRecord = mbis.str();
}

void ASTMutationPoint::performMutation(ASTMutator &mutator) {
  mutation->performMutation(*this, mutator);
}

} // namespace cxx
} // namespace mull

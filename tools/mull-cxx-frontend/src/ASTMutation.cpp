#include "ASTMutation.h"

#include <clang/AST/Stmt.h>

#include <sstream>
#include <unordered_map>

ASTMutation::ASTMutation(std::unique_ptr<Mutator> mutator,
                         mull::MutatorKind mutationType,
                         std::string mutationIdentifier,
                         clang::Stmt *toBeMutatedStmt, std::string sourceFilePath, int line,
                         int column)
    : mutator(std::move(mutator)), mutationType(mutationType), mutableStmt(toBeMutatedStmt),
      sourceFilePath(sourceFilePath), line(line), column(column) {
  std::ostringstream mis;

  /// mutator:file:line:col:1
  mis << mutationIdentifier << ":" << sourceFilePath << ":" << line << ":" << column;
  this->mutationIdentifier = mis.str();
}

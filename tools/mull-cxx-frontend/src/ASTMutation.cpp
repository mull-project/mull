#include "ASTMutation.h"

#include <clang/AST/Stmt.h>

#include <sstream>
#include <unordered_map>

static const std::unordered_map<mull::MutatorKind, std::string> KINDS_TO_STRINGS = {
  { mull::MutatorKind::CXX_AddToSub, "cxx_add_to_sub" },
  { mull::MutatorKind::CXX_Logical_OrToAnd, "cxx_logical_or_to_and" },
  { mull::MutatorKind::CXX_RemoveVoidCall, "cxx_remove_void_call" },
};

ASTMutation::ASTMutation(mull::MutatorKind mutationType, clang::Stmt *toBeMutatedStmt,
                         std::string sourceFilePath, int line, int column)
    : mutationType(mutationType), mutableStmt(toBeMutatedStmt), sourceFilePath(sourceFilePath),
      line(line), column(column) {
  std::ostringstream mis;

  assert(KINDS_TO_STRINGS.count(mutationType) > 0);

  /// mutator:file:line:col:1
  mis << KINDS_TO_STRINGS.at(mutationType) << ":" << sourceFilePath << ":" << line << ":" << column
      << ":"
      << "1";

  this->mutationIdentifier = mis.str();
}

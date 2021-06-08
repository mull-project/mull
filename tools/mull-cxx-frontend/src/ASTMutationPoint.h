#pragma once

#include "ASTMutation.h"

#include "mull/Mutators/MutatorKind.h"

namespace clang {
class Stmt;
}

namespace mull {
namespace cxx {

class ASTMutationPoint {
public:
  std::unique_ptr<ASTMutation> mutation;
  mull::MutatorKind mutationType;
  clang::Stmt *mutableStmt;
  std::string sourceFilePath;
  int beginLine;
  int beginColumn;
  int endLine;
  int endColumn;
  std::string mutationIdentifier;
  std::string mutationBinaryRecord;
  ASTMutationPoint(std::unique_ptr<ASTMutation> mutation, mull::MutatorKind mutationType,
                   std::string mutationIdentifier, clang::Stmt *toBeMutatedStmt,
                   std::string sourceFilePath, int beginLine, int beginColumn, int endLine,
                   int endColumn);

  void performMutation(ASTMutator &mutator);
};

} // namespace cxx
} // namespace mull

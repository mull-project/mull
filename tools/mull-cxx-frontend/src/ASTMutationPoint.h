#pragma once

#include "ASTMutation.h"

#include "mull/Mutators/MutatorKind.h"

namespace clang {
class Stmt;
}

class ASTMutationPoint {
public:
  std::unique_ptr<ASTMutation> mutation;
  mull::MutatorKind mutationType;
  clang::Stmt *mutableStmt;
  std::string sourceFilePath;
  int line;
  int column;
  std::string mutationIdentifier;
  ASTMutationPoint(std::unique_ptr<ASTMutation> mutation, mull::MutatorKind mutationType,
                   std::string mutationIdentifier, clang::Stmt *toBeMutatedStmt,
                   std::string sourceFilePath, int line, int column);

  void performMutation(ASTMutator &mutator);
};

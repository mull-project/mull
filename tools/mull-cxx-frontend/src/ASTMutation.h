#pragma once

#include "ASTMutator.h"

#include "mull/Mutators/MutatorKind.h"

namespace clang {
class Stmt;
}

class ASTMutation {
public:
  std::unique_ptr<Mutator> mutator;
  mull::MutatorKind mutationType;
  clang::Stmt *mutableStmt;
  std::string sourceFilePath;
  int line;
  int column;
  std::string mutationIdentifier;
  ASTMutation(std::unique_ptr<Mutator> mutator, mull::MutatorKind mutationType,
              std::string mutationIdentifier, clang::Stmt *toBeMutatedStmt,
              std::string sourceFilePath, int line, int column);
};

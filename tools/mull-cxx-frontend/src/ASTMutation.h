#pragma once

#include "mull/Mutators/MutatorKind.h"

namespace clang {
  class Stmt;
}

struct ASTMutation {
  mull::MutatorKind mutationType;
  clang::Stmt *mutableStmt;
  std::string sourceFilePath;
  int line;
  int column;
  std::string mutationIdentifier;
  ASTMutation(mull::MutatorKind mutationType, clang::Stmt *toBeMutatedStmt,
              std::string sourceFilePath, int line, int column);
};

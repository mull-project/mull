#pragma once

#include "mull/Mutators/MutatorKind.h"

#include <clang/AST/Stmt.h>

struct ASTMutation {
  mull::MutatorKind mutationType;
  clang::Stmt *mutableStmt;
  int line;
  int column;
  ASTMutation(mull::MutatorKind mutationType, clang::Stmt *toBeMutatedStmt, int line, int column)
      : mutationType(mutationType), mutableStmt(toBeMutatedStmt), line(line), column(column) {}
};

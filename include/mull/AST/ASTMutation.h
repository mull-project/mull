#pragma once

#include <mull/Mutators/Mutator.h>

namespace clang {
  class Stmt;
}

namespace mull {

struct ASTMutation {
  mull::MutatorKind mutatorKind;
  int line;
  int column;
  const clang::Stmt *const stmt;
  ASTMutation(MutatorKind mutatorKind, int line, int column, const clang::Stmt *const stmt)
      : mutatorKind(mutatorKind), line(line), column(column), stmt(stmt) {}
};

} // namespace mull

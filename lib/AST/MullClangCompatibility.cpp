#include "mull/AST/MullClangCompatibility.h"

#include <clang/AST/Stmt.h>

clang::SourceLocation ClangCompatibilityStmtGetBeginLoc(const clang::Stmt &stmt) {
  return stmt.getBeginLoc();
}

#include "mull/AST/MullClangCompatibility.h"

#include <clang/AST/Stmt.h>

clang::SourceLocation ClangCompatibilityStmtGetBeginLoc(const clang::Stmt &stmt) {
#if LLVM_VERSION_MAJOR >= 7
  return stmt.getBeginLoc();
#else
  return stmt.getLocStart();
#endif
}

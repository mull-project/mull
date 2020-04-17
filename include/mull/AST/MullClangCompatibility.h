#include <clang/Basic/SourceLocation.h>

namespace clang {
  class Stmt;
};

clang::SourceLocation ClangCompatibilityStmtGetBeginLoc(const clang::Stmt &stmt);

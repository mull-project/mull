#include <clang/Basic/SourceLocation.h>

#if LLVM_VERSION_MAJOR >= 11
#include <clang/AST/ParentMapContext.h>
#endif

namespace clang {
  class Stmt;
};

clang::SourceLocation ClangCompatibilityStmtGetBeginLoc(const clang::Stmt &stmt);

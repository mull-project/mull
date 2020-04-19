#include "mull/AST/ASTScalarMutationMatcher.h"

#include "mull/AST/MullClangCompatibility.h"
#include "mull/Program/Program.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceLocation.h>

using namespace mull;

static bool isConstant(const clang::Stmt &statement) {
  if (clang::isa<clang::IntegerLiteral>(&statement)) {
    return true;
  }
  return false;
}

ASTScalarMutationMatcher::ASTScalarMutationMatcher(clang::ASTContext &context)
    : context(context) {}

bool ASTScalarMutationMatcher::isMutableExpr(const clang::Expr &expr,
                                             const clang::Stmt **mutableParentStmt,
                                             clang::SourceLocation *mutationLocation) {
  if (isConstant(expr) == false) {
    return false;
  }

  bool foundMutableParent = findMutableParentStmt(expr, mutableParentStmt, mutationLocation);
  if (foundMutableParent) {
    return true;
  }

  return false;
}

bool ASTScalarMutationMatcher::findMutableParentStmt(const clang::Stmt &statement,
                                                     const clang::Stmt **mutableParentStmt,
                                                     clang::SourceLocation *mutationLocation) {
  for (auto &parent : context.getParents(statement)) {
    if (const clang::ReturnStmt *returnParent = parent.get<clang::ReturnStmt>()) {
      if (mutableParentStmt) {
        *mutableParentStmt = returnParent;
      }
      if (mutationLocation) {
        *mutationLocation = ClangCompatibilityStmtGetBeginLoc(*returnParent);
      }
      return true;
    }
  }
  return false;
}

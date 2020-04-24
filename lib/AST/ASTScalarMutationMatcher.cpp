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

ASTScalarMutationMatcher::ASTScalarMutationMatcher(clang::ASTContext &context) : context(context) {}

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

  /// Please keep in mind that the method used for obtaining mutationLocation
  /// can be different for each parent statement depending on its type.
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
    if (const clang::BinaryOperator *binaryOpParent = parent.get<clang::BinaryOperator>()) {
      if (mutableParentStmt) {
        *mutableParentStmt = binaryOpParent;
      }
      if (mutationLocation) {
        *mutationLocation = binaryOpParent->getOperatorLoc();
      }
      return true;
    }
    if (const clang::VarDecl *varDecl = parent.get<clang::VarDecl>()) {
      if (varDecl->getType().isConstQualified() == false) {
        if (mutationLocation) {
          *mutationLocation = varDecl->getLocation();
        }
        return true;
      }
    }
    if (const clang::CallExpr *callExpr = parent.get<clang::CallExpr>()) {
      if (mutationLocation) {
        *mutationLocation = ClangCompatibilityStmtGetBeginLoc(*callExpr);
      }
      return true;
    }
  }
  return false;
}

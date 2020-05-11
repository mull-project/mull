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

  /// Please keep in mind that the method used for obtaining mutationLocation
  /// can be different for each parent statement depending on its type.
  for (auto &parent : context.getParents(expr)) {
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
  }

  return false;
}

bool ASTScalarMutationMatcher::isMutableVarDeclExpr(const clang::Expr &expr,
                                                    clang::SourceLocation *mutationLocation) {
  if (isConstant(expr) == false) {
    return false;
  }

  for (auto &parent : context.getParents(expr)) {
    if (const clang::VarDecl *varDecl = parent.get<clang::VarDecl>()) {
      if (varDecl->getType().isConstQualified() == false) {
        if (mutationLocation) {
          *mutationLocation = varDecl->getLocation();
        }
        return true;
      }
    }
  }

  return false;
}

bool ASTScalarMutationMatcher::isMutableAssignmentExpr(const clang::Expr &expr,
                                                       clang::SourceLocation *mutationLocation) {
  if (isConstant(expr) == false) {
    return false;
  }

  for (auto &parent : context.getParents(expr)) {
    if (const clang::BinaryOperator *binaryOperator = parent.get<clang::BinaryOperator>()) {
      if (binaryOperator->getOpcode() == clang::BinaryOperator::Opcode::BO_Assign) {
        if (mutationLocation) {
          *mutationLocation = binaryOperator->getOperatorLoc();
        }
        return true;
      }
    }
  }

  return false;
}

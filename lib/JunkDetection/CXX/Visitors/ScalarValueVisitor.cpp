#include "mull/JunkDetection/CXX/Visitors/ScalarValueVisitor.h"

using namespace mull;

static bool isDeclRefConstant(clang::DeclRefExpr *declRef);

/// A statement is constant if it is either:
///  - an integer literal
///  - a floating literal
///  - a reference to a constant variable declaration
/// A variable declaration is constant if all of its children are constants

static bool isConstant(clang::Stmt *statement) {
  if (llvm::isa<clang::IntegerLiteral>(statement)) {
    return true;
  }
  if (llvm::isa<clang::FloatingLiteral>(statement)) {
    return true;
  }

  if (auto declRef = llvm::dyn_cast<clang::DeclRefExpr>(statement)) {
    return isDeclRefConstant(declRef);
  }

  return false;
}

bool isDeclRefConstant(clang::DeclRefExpr *declRef) {
  auto varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
  if (!varDecl) {
    return false;
  }
  if (!varDecl->hasInit()) {
    return false;
  }

  clang::Expr *init = varDecl->getInit();
  if (isConstant(init->IgnoreImplicit())) {
    return true;
  }

  for (clang::Stmt *child : init->children()) {
    if (auto expr = llvm::dyn_cast<clang::Expr>(child)) {
      child = expr->IgnoreImplicit();
    }
    if (!isConstant(child)) {
      return false;
    }
  }

  return true;
}

ScalarValueVisitor::ScalarValueVisitor(const VisitorParameters &parameters)
    : mutantStmt(nullptr), scalarMutationMatcher(parameters.astContext) {}

bool ScalarValueVisitor::VisitExpr(clang::Expr *expression) {
  const clang::Stmt *potentialMutableStatement = nullptr;
  if (scalarMutationMatcher.isMutableExpr(*expression, &potentialMutableStatement, nullptr)) {
    mutantStmt = expression;
  }
  return true;
}

const clang::Stmt *ScalarValueVisitor::foundMutant() {
  return mutantStmt;
}

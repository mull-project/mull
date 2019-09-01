#include "mull/JunkDetection/CXX/Visitors/ScalarValueVisitor.h"

using namespace mull;

static bool isDeclRefConstant(clang::DeclRefExpr *declRef);

/// A statement is constant if it is either:
///  - an integer literal
///  - a floating literal
///  - a reference to a constant variable declaration
/// A variable declaration is constant if all of its children are constants

static bool isConstant(clang::Stmt *statement) {
  statement = statement->IgnoreImplicit();
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

  auto init = varDecl->getInit();
  if (isConstant(init)) {
    return true;
  }

  for (auto child : init->children()) {
    if (!isConstant(child)) {
      return false;
    }
  }

  return true;
}

ScalarValueVisitor::ScalarValueVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool ScalarValueVisitor::VisitExpr(clang::Expr *expr) {

  /// In case of PredefinedExpr, the expression expr->children() yields
  /// children which are nullptr. These nodes should be junk anyway, so we
  /// ignore them early.
  if (llvm::isa<clang::PredefinedExpr>(expr)) {
    return true;
  }

  expr = expr->IgnoreImplicit();
  for (auto child : expr->children()) {
    if (!child) {
      llvm::errs() << "Ignoring expression with a nullptr child:" << "\n";
      expr->dump(llvm::errs());
      continue;
    }

    child = child->IgnoreImplicit();
    if (isConstant(child)) {
      visitor.visitRangeWithASTExpr(expr);
    }
  }
  return true;
}

clang::Expr *ScalarValueVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

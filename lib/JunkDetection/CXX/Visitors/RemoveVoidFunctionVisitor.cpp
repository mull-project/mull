#include "mull/JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.h"

using namespace mull;

RemoveVoidFunctionVisitor::RemoveVoidFunctionVisitor(
    const VisitorParameters &parameters)
    : visitor(parameters), astContext(parameters.astContext) {}

bool RemoveVoidFunctionVisitor::VisitCallExpr(clang::CallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

bool RemoveVoidFunctionVisitor::VisitCXXMemberCallExpr(
    clang::CXXMemberCallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

bool RemoveVoidFunctionVisitor::VisitCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

void RemoveVoidFunctionVisitor::handleCallExpr(
    clang::CallExpr *callExpression) {
  auto *type = callExpression->getType().getTypePtrOrNull();
  if (type && type->isVoidType()) {
    visitor.visitRangeWithASTExpr(callExpression);
  }
}

clang::Expr *RemoveVoidFunctionVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

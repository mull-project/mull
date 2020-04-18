#include "mull/JunkDetection/CXX/Visitors/ReplaceCallVisitor.h"

using namespace mull;

ReplaceCallVisitor::ReplaceCallVisitor(const VisitorParameters &parameters) : visitor(parameters) {}

bool ReplaceCallVisitor::VisitCallExpr(clang::CallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

bool ReplaceCallVisitor::VisitCXXMemberCallExpr(clang::CXXMemberCallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

bool ReplaceCallVisitor::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr *callExpression) {
  handleCallExpr(callExpression);
  return true;
}

void ReplaceCallVisitor::handleCallExpr(clang::CallExpr *callExpression) {
  auto *type = callExpression->getType().getTypePtrOrNull();
  /// Real Type = float, double, long double, integer
  if (type && type->isRealType()) {
    visitor.visitRangeWithASTExpr(callExpression);
  }
}

const clang::Stmt *ReplaceCallVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

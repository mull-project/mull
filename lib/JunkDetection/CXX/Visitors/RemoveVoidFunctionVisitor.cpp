#include "mull/JunkDetection/CXX/Visitors/RemoveVoidFunctionVisitor.h"

using namespace mull;

static bool isVoidCall(clang::CallExpr *callExpression) {
  auto *type = callExpression->getType().getTypePtrOrNull();
  return type && type->isVoidType();
}

RemoveVoidFunctionVisitor::RemoveVoidFunctionVisitor(const VisitorParameters &parameters)
    : parameters(parameters), matchingExpression(nullptr) {}

bool RemoveVoidFunctionVisitor::VisitCallExpr(clang::CallExpr *callExpression) {
  return !validCallExpr(callExpression,
                        callExpression->getSourceRange().getBegin());
}

bool RemoveVoidFunctionVisitor::VisitCXXMemberCallExpr(
    clang::CXXMemberCallExpr *callExpression) {
  return !validCallExpr(callExpression, callExpression->getExprLoc());
}

bool RemoveVoidFunctionVisitor::VisitCXXOperatorCallExpr(
    clang::CXXOperatorCallExpr *callExpression) {
  return !validCallExpr(callExpression, callExpression->getOperatorLoc());
}

bool RemoveVoidFunctionVisitor::validCallExpr(clang::CallExpr *callExpression,
                                              clang::SourceLocation location) {
  if (isVoidCall(callExpression) && location == parameters.sourceLocation) {
    assert(!matchingExpression && "Already found");
    matchingExpression = callExpression;
    return true;
  }
  return false;
}

clang::Expr *RemoveVoidFunctionVisitor::foundMutant() {
  return matchingExpression;
}

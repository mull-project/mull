#include "mull/JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"

using namespace mull;

ConditionalsBoundaryVisitor::ConditionalsBoundaryVisitor(
    const VisitorParameters &parameters)
    : visitor(parameters) {}

bool ConditionalsBoundaryVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (!binaryOperator->isRelationalOp()) {
    return true;
  }

  visitor.visitRangeWithASTExpr(binaryOperator);
  return true;
}

clang::Expr *ConditionalsBoundaryVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

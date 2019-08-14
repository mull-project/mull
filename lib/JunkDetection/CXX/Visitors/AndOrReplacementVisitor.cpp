#include "mull/JunkDetection/CXX/Visitors/AndOrReplacementVisitor.h"

using namespace mull;

AndOrReplacementVisitor::AndOrReplacementVisitor(
    const VisitorParameters &parameters)
    : visitor(parameters) {}

bool AndOrReplacementVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (!binaryOperator->isLogicalOp()) {
    return true;
  }

  visitor.visitRangeWithASTExpr(binaryOperator);
  return true;
}

clang::Expr *AndOrReplacementVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

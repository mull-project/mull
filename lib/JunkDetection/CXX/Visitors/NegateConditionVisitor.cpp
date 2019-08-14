#include "mull/JunkDetection/CXX/Visitors/NegateConditionVisitor.h"

using namespace mull;

NegateConditionVisitor::NegateConditionVisitor(
    const VisitorParameters &parameters)
    : visitor(parameters) {}

bool NegateConditionVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->isRelationalOp() || binaryOperator->isEqualityOp()) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }
  return true;
}

bool NegateConditionVisitor::VisitUnaryOperator(
    clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->getOpcode() == clang::UnaryOperatorKind::UO_LNot) {
    visitor.visitRangeWithASTExpr(unaryOperator);
  }
  return true;
}

clang::Expr *NegateConditionVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

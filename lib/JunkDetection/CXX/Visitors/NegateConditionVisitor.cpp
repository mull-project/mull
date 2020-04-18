#include "mull/JunkDetection/CXX/Visitors/NegateConditionVisitor.h"

using namespace mull;

NegateConditionVisitor::NegateConditionVisitor(
    const VisitorParameters &parameters)
    : visitor(parameters) {}

bool NegateConditionVisitor::VisitUnaryOperator(
    clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->getOpcode() == clang::UnaryOperatorKind::UO_LNot) {
    visitor.visitRangeWithASTExpr(unaryOperator);
  }
  return true;
}

const clang::Stmt *NegateConditionVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

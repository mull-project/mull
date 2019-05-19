#include "mull/JunkDetection/CXX/Visitors/MathDivVisitor.h"

using namespace mull;

MathDivVisitor::MathDivVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathDivVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Div) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_DivAssign) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }

  return true;
}

clang::Expr *MathDivVisitor::foundMutant() { return visitor.getMatchingASTNode(); }

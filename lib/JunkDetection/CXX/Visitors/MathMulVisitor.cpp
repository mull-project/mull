#include "mull/JunkDetection/CXX/Visitors/MathMulVisitor.h"

using namespace mull;

MathMulVisitor::MathMulVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathMulVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Mul) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_MulAssign) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }

  return true;
}

clang::Expr *MathMulVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

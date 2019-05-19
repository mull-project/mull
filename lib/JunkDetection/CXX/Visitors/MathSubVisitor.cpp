#include "mull/JunkDetection/CXX/Visitors/MathSubVisitor.h"

using namespace mull;

MathSubVisitor::MathSubVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathSubVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Sub) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_SubAssign) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }

  return true;
}

bool MathSubVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->isDecrementOp()) {
    visitor.visitRangeWithASTExpr(unaryOperator);
  }

  return true;
}

clang::Expr *MathSubVisitor::foundMutant() { return visitor.getMatchingASTNode(); }

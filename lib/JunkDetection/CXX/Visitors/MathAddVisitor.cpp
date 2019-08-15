#include "mull/JunkDetection/CXX/Visitors/MathAddVisitor.h"

using namespace mull;

MathAddVisitor::MathAddVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathAddVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  auto range = binaryOperator->getSourceRange();
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Add) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_AddAssign) {
    visitor.visitRangeWithASTExpr(binaryOperator);
  }

  return true;
}

bool MathAddVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->isIncrementOp()) {
    visitor.visitRangeWithASTExpr(unaryOperator);
  }

  return true;
}

clang::Expr *MathAddVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

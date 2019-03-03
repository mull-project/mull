#include "JunkDetection/CXX/Visitors/MathSubVisitor.h"

using namespace mull;

MathSubVisitor::MathSubVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathSubVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  auto range = binaryOperator->getSourceRange();
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Sub) {
    visitor.visitRangeWithLocation(range);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_SubAssign) {
    visitor.visitRangeWithLocation(range);
  }

  return true;
}

bool MathSubVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->isDecrementOp()) {
    visitor.visitRangeWithLocation(unaryOperator->getSourceRange());
  }

  return true;
}

bool MathSubVisitor::foundMutant() { return visitor.foundRange(); }

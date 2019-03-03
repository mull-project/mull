#include "JunkDetection/CXX/Visitors/MathAddVisitor.h"

using namespace mull;

MathAddVisitor::MathAddVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathAddVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  auto range = binaryOperator->getSourceRange();
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Add) {
    visitor.visitRangeWithLocation(range);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_AddAssign) {
    visitor.visitRangeWithLocation(range);
  }

  return true;
}

bool MathAddVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->isIncrementOp()) {
    visitor.visitRangeWithLocation(unaryOperator->getSourceRange());
  }

  return true;
}

bool MathAddVisitor::foundMutant() { return visitor.foundRange(); }

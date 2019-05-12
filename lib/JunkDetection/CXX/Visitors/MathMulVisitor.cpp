#include "mull/JunkDetection/CXX/Visitors/MathMulVisitor.h"

using namespace mull;

MathMulVisitor::MathMulVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathMulVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  auto range = binaryOperator->getSourceRange();
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Mul) {
    visitor.visitRangeWithLocation(range);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_MulAssign) {
    visitor.visitRangeWithLocation(range);
  }

  return true;
}

bool MathMulVisitor::foundMutant() { return visitor.foundRange(); }

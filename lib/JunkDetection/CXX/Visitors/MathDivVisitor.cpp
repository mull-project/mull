#include "mull/JunkDetection/CXX/Visitors/MathDivVisitor.h"

using namespace mull;

MathDivVisitor::MathDivVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool MathDivVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  auto range = binaryOperator->getSourceRange();
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_Div) {
    visitor.visitRangeWithLocation(range);
  }
  if (binaryOperator->getOpcode() == clang::BinaryOperatorKind::BO_DivAssign) {
    visitor.visitRangeWithLocation(range);
  }

  return true;
}

bool MathDivVisitor::foundMutant() { return visitor.foundRange(); }

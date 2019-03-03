#include "JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"

using namespace mull;

ConditionalsBoundaryVisitor::ConditionalsBoundaryVisitor(
    const VisitorParameters &parameters)
    : visitor(parameters) {}

bool ConditionalsBoundaryVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (!binaryOperator->isRelationalOp()) {
    return true;
  }

  visitor.visitRangeWithLocation(binaryOperator->getSourceRange());
  return true;
}

bool ConditionalsBoundaryVisitor::foundMutant() { return visitor.foundRange(); }

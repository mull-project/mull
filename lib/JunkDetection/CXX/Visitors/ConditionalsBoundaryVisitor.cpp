#include "JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"

mull::ConditionalsBoundaryVisitor::ConditionalsBoundaryVisitor(
    const clang::SourceManager &sourceManager,
    const clang::SourceLocation &sourceLocation)
    : visitor(sourceManager, sourceLocation) {}

bool mull::ConditionalsBoundaryVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (!binaryOperator->isRelationalOp()) {
    return true;
  }

  visitor.visitRangeWithLocation(binaryOperator->getSourceRange());
  return true;
}

bool mull::ConditionalsBoundaryVisitor::foundMutant() {
  return visitor.foundRange();
}

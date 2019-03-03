#include "JunkDetection/CXX/Visitors/ConditionalsBoundaryVisitor.h"

using namespace mull;

ConditionalsBoundaryVisitor::ConditionalsBoundaryVisitor(
    const clang::SourceManager &sourceManager,
    const clang::SourceLocation &sourceLocation)
    : visitor(sourceManager, sourceLocation) {}

bool ConditionalsBoundaryVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (!binaryOperator->isRelationalOp()) {
    return true;
  }

  visitor.visitRangeWithLocation(binaryOperator->getSourceRange());
  return true;
}

bool ConditionalsBoundaryVisitor::foundMutant() { return visitor.foundRange(); }

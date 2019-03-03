#include "JunkDetection/CXX/Visitors/NegateConditionVisitor.h"

using namespace mull;

NegateConditionVisitor::NegateConditionVisitor(
    const clang::SourceManager &sourceManager,
    const clang::SourceLocation &sourceLocation)
    : visitor(sourceManager, sourceLocation) {}

bool NegateConditionVisitor::VisitBinaryOperator(
    clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->isRelationalOp() || binaryOperator->isEqualityOp()) {
    visitor.visitRangeWithLocation(binaryOperator->getSourceRange());
  }
  return true;
}

bool NegateConditionVisitor::VisitUnaryOperator(
    clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->getOpcode() == clang::UnaryOperatorKind::UO_LNot) {
    visitor.visitRangeWithLocation(unaryOperator->getSourceRange());
  }
  return true;
}

bool NegateConditionVisitor::foundMutant() { return visitor.foundRange(); }

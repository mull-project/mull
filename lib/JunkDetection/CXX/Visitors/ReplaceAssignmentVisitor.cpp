#include "mull/JunkDetection/CXX/Visitors/ReplaceAssignmentVisitor.h"

using namespace mull;

ReplaceAssignmentVisitor::ReplaceAssignmentVisitor(const VisitorParameters &parameters)
    : visitor(parameters) {}

bool ReplaceAssignmentVisitor::VisitBinAssign(clang::BinaryOperator *binaryOperator) {
  visitor.visitRangeWithASTExpr(binaryOperator);
  return true;
}

clang::Expr *ReplaceAssignmentVisitor::foundMutant() {
  return visitor.getMatchingASTNode();
}

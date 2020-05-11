#include "mull/JunkDetection/CXX/Visitors/UnaryVisitor.h"

using namespace mull;

clang::Expr *cxx::UnaryVisitor::findMutant(const VisitorParameters &parameters,
                                           clang::UnaryOperator::Opcode opcode, clang::Decl *decl) {
  UnaryVisitor visitor(parameters, opcode);
  visitor.TraverseDecl(decl);
  return visitor.foundMutant();
}

cxx::UnaryVisitor::UnaryVisitor(const VisitorParameters &parameters,
                                clang::UnaryOperator::Opcode opcode)
    : parameters(parameters), matchingExpression(nullptr), opcode(opcode) {}

bool cxx::UnaryVisitor::VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
  if (unaryOperator->getOpcode() != opcode) {
    return true;
  }

  if (unaryOperator->getOperatorLoc() == parameters.sourceLocation) {
    assert(!matchingExpression && "Already found");
    matchingExpression = unaryOperator;
    return false;
  }
  return true;
}

clang::Expr *cxx::UnaryVisitor::foundMutant() {
  return matchingExpression;
}
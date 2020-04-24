#include "mull/JunkDetection/CXX/Visitors/BinaryVisitor.h"

using namespace mull;

clang::Expr *cxx::BinaryVisitor::findMutant(const VisitorParameters &parameters, clang::Decl *decl,
                                            clang::BinaryOperator::Opcode opcode) {
  BinaryVisitor visitor(parameters, opcode);
  visitor.TraverseDecl(decl);
  return visitor.foundMutant();
}

cxx::BinaryVisitor::BinaryVisitor(const VisitorParameters &parameters,
                                  clang::BinaryOperator::Opcode opcode)
    : parameters(parameters), matchingExpression(nullptr), opcode(opcode) {}

bool cxx::BinaryVisitor::VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
  if (binaryOperator->getOpcode() != opcode) {
    return true;
  }

  if (binaryOperator->getOperatorLoc() == parameters.sourceLocation) {
    assert(!matchingExpression && "Already found");
    matchingExpression = binaryOperator;
    return false;
  }
  return true;
}

clang::Expr *cxx::BinaryVisitor::foundMutant() {
  return matchingExpression;
}

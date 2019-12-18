#pragma once

#include "VisitorParameters.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <assert.h>

namespace mull {

namespace cxx {

template <clang::UnaryOperator::Opcode opcode>
class UnaryVisitor : public clang::RecursiveASTVisitor<UnaryVisitor<opcode>> {
public:
  explicit UnaryVisitor(const VisitorParameters &parameters)
      : parameters(parameters), matchingExpression(nullptr) {}

  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
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

  clang::Expr *foundMutant() { return matchingExpression; }

private:
  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
};

typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_PreInc> PreIncVisitor;
typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_PostInc> PostIncVisitor;
typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_PreDec> PreDecVisitor;
typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_PostDec> PostDecVisitor;
typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_Not> BitwiseNotVisitor;
typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_Minus> UnaryMinusVisitor;

} // namespace cxx

} // namespace mull

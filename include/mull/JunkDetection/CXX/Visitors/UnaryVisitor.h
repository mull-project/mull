#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

namespace cxx {

template <clang::UnaryOperator::Opcode opcode>
class UnaryVisitor : public clang::RecursiveASTVisitor<UnaryVisitor<opcode>> {
public:
  UnaryVisitor(const VisitorParameters &parameters) : visitor(parameters) {}

  bool VisitUnaryOperator(clang::UnaryOperator *unaryOperator) {
    if (unaryOperator->getOpcode() != opcode) {
      return true;
    }

    visitor.visitRangeWithASTExpr(unaryOperator);
    return true;
  }

  clang::Expr *foundMutant() { return visitor.getMatchingASTNode(); }

private:
  InstructionRangeVisitor visitor;
};

typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_PreInc> PreIncVisitor;
typedef UnaryVisitor<clang::UnaryOperator::Opcode::UO_PostInc> PostIncVisitor;

} // namespace cxx

} // namespace mull

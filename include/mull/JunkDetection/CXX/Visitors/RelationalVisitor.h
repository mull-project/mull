#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

namespace cxx {

template <clang::BinaryOperator::Opcode opcode>
class RelationalVisitor
    : public clang::RecursiveASTVisitor<RelationalVisitor<opcode>> {
public:
  RelationalVisitor(const VisitorParameters &parameters)
      : visitor(parameters) {}

  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
    if (binaryOperator->getOpcode() != opcode) {
      return true;
    }

    visitor.visitRangeWithASTExpr(binaryOperator);
    return true;
  }

  clang::Expr *foundMutant() { return visitor.getMatchingASTNode(); }

private:
  InstructionRangeVisitor visitor;
};

typedef RelationalVisitor<clang::BinaryOperator::Opcode::BO_LT>
    LessThanToLessOrEqualVisitor;
typedef RelationalVisitor<clang::BinaryOperator::Opcode::BO_LE>
    LessOrEqualToLessThanVisitor;
typedef RelationalVisitor<clang::BinaryOperator::Opcode::BO_GT>
    GreaterThanToGreaterOrEqualVisitor;
typedef RelationalVisitor<clang::BinaryOperator::Opcode::BO_GE>
    GreaterOrEqualToGreaterThanVisitor;

} // namespace cxx

} // namespace mull

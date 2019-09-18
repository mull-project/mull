#pragma once

#include "InstructionRangeVisitor.h"
#include "VisitorParameters.h"

#include <clang/AST/RecursiveASTVisitor.h>

namespace mull {

namespace cxx {

template <clang::BinaryOperator::Opcode opcode>
class BinaryVisitor : public clang::RecursiveASTVisitor<BinaryVisitor<opcode>> {
public:
  BinaryVisitor(const VisitorParameters &parameters) : visitor(parameters) {}

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

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_LT> LessThanVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_LE> LessOrEqualVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_GT> GreaterThanVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_GE>
    GreaterOrEqualVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_EQ> EqualVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_NE> NotEqualVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Add> AddVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_AddAssign>
    AddAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Sub> SubVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_SubAssign>
    SubAssignVisitor;

} // namespace cxx

} // namespace mull

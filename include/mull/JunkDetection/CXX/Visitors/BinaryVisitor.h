#pragma once

#include "VisitorParameters.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <assert.h>

namespace mull {

namespace cxx {

template <clang::BinaryOperator::Opcode opcode>
class BinaryVisitor : public clang::RecursiveASTVisitor<BinaryVisitor<opcode>> {
public:
  explicit BinaryVisitor(const VisitorParameters &parameters)
      : parameters(parameters), matchingExpression(nullptr) {}

  bool VisitBinaryOperator(clang::BinaryOperator *binaryOperator) {
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

  clang::Expr *foundMutant() {
    return matchingExpression;
  }

private:
  const VisitorParameters &parameters;
  clang::Expr *matchingExpression;
};

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_LT> LessThanVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_LE> LessOrEqualVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_GT> GreaterThanVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_GE> GreaterOrEqualVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_EQ> EqualVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_NE> NotEqualVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Add> AddVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_AddAssign> AddAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Sub> SubVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_SubAssign> SubAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Mul> MulVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_MulAssign> MulAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Div> DivVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_DivAssign> DivAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Rem> RemVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_RemAssign> RemAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Shl> LShiftVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_ShlAssign> LShiftAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Shr> RShiftVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_ShrAssign> RShiftAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_And> BitAndVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_AndAssign> BitAndAssignVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Or> BitOrVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_OrAssign> BitOrAssignVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Xor> BitXorVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_XorAssign> BitXorAssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_Assign> AssignVisitor;

typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_LAnd> LogicalAndVisitor;
typedef BinaryVisitor<clang::BinaryOperator::Opcode::BO_LOr> LogicalOrVisitor;

} // namespace cxx

} // namespace mull

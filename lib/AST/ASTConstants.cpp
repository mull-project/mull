#include "mull/AST/ASTConstants.h"

std::vector<std::pair<clang::BinaryOperator::Opcode, mull::MutatorKind>> mull::BINARY_MUTATIONS = {
  { clang::BO_Add, MutatorKind::CXX_AddToSub },
  { clang::BO_Sub, MutatorKind::CXX_SubToAdd },
  { clang::BO_Mul, MutatorKind::CXX_MulToDiv },
  { clang::BO_Div, MutatorKind::CXX_DivToMul },
  { clang::BO_Rem, MutatorKind::CXX_RemToDiv },

  { clang::BO_AddAssign, MutatorKind::CXX_AddAssignToSubAssign },
  { clang::BO_SubAssign, MutatorKind::CXX_SubAssignToAddAssign },
  { clang::BO_MulAssign, MutatorKind::CXX_MulAssignToDivAssign },
  { clang::BO_DivAssign, MutatorKind::CXX_DivAssignToMulAssign },
  { clang::BO_RemAssign, MutatorKind::CXX_RemAssignToDivAssign },

  { clang::BO_EQ, MutatorKind::CXX_EqualToNotEqual },
  { clang::BO_NE, MutatorKind::CXX_NotEqualToEqual },
  { clang::BO_GT, MutatorKind::CXX_GreaterThanToLessOrEqual },
  { clang::BO_LT, MutatorKind::CXX_LessThanToGreaterOrEqual },
  { clang::BO_GE, MutatorKind::CXX_GreaterOrEqualToLessThan },
  { clang::BO_LE, MutatorKind::CXX_LessOrEqualToGreaterThan },

  { clang::BO_GT, MutatorKind::CXX_GreaterThanToGreaterOrEqual },
  { clang::BO_LT, MutatorKind::CXX_LessThanToLessOrEqual },
  { clang::BO_GE, MutatorKind::CXX_GreaterOrEqualToGreaterThan },
  { clang::BO_LE, MutatorKind::CXX_LessOrEqualToLessThan },

  { clang::BO_And, MutatorKind::CXX_Bitwise_AndToOr },
  { clang::BO_Or, MutatorKind::CXX_Bitwise_OrToAnd },
  { clang::BO_Xor, MutatorKind::CXX_Bitwise_XorToOr },
  { clang::BO_Shl, MutatorKind::CXX_LShiftToRShift },
  { clang::BO_Shr, MutatorKind::CXX_RShiftToLShift },

  { clang::BO_AndAssign, MutatorKind::CXX_Bitwise_AndAssignToOrAssign },
  { clang::BO_OrAssign, MutatorKind::CXX_Bitwise_OrAssignToAndAssign },
  { clang::BO_XorAssign, MutatorKind::CXX_Bitwise_XorAssignToOrAssign },
  { clang::BO_ShlAssign, MutatorKind::CXX_LShiftAssignToRShiftAssign },
  { clang::BO_ShrAssign, MutatorKind::CXX_RShiftAssignToLShiftAssign },

  { clang::BO_LAnd, MutatorKind::CXX_Logical_AndToOr },
  { clang::BO_LOr, MutatorKind::CXX_Logical_OrToAnd }
};

std::vector<std::pair<clang::UnaryOperator::Opcode, mull::MutatorKind>> mull::UNARY_MUTATIONS = {
  { clang::UO_LNot, MutatorKind::CXX_RemoveNegation },
  { clang::UO_Minus, MutatorKind::CXX_UnaryMinusToNoop }
};

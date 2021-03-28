#pragma once

#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

namespace mull {

enum class MutatorKind {
  InvalidKind,

  NegateMutator,
  ScalarValueMutator,

  CXX_RemoveVoidCall,
  CXX_ReplaceScalarCall,

  CXX_AddToSub,
  CXX_AddAssignToSubAssign,
  CXX_PreIncToPreDec,
  CXX_PostIncToPostDec,

  CXX_SubToAdd,
  CXX_SubAssignToAddAssign,
  CXX_PreDecToPreInc,
  CXX_PostDecToPostInc,

  CXX_MulToDiv,
  CXX_MulAssignToDivAssign,

  CXX_DivToMul,
  CXX_DivAssignToMulAssign,

  CXX_RemToDiv,
  CXX_RemAssignToDivAssign,

  CXX_BitwiseNotToNoop,
  CXX_UnaryMinusToNoop,

  CXX_LShiftToRShift,
  CXX_LShiftAssignToRShiftAssign,

  CXX_RShiftToLShift,
  CXX_RShiftAssignToLShiftAssign,

  CXX_Logical_AndToOr,
  CXX_Logical_OrToAnd,

  CXX_Bitwise_OrToAnd,
  CXX_Bitwise_OrAssignToAndAssign,
  CXX_Bitwise_AndToOr,
  CXX_Bitwise_AndAssignToOrAssign,
  CXX_Bitwise_XorToOr,
  CXX_Bitwise_XorAssignToOrAssign,

  CXX_LessThanToLessOrEqual,
  CXX_LessOrEqualToLessThan,
  CXX_GreaterThanToGreaterOrEqual,
  CXX_GreaterOrEqualToGreaterThan,

  CXX_GreaterThanToLessOrEqual,
  CXX_GreaterOrEqualToLessThan,
  CXX_LessThanToGreaterOrEqual,
  CXX_LessOrEqualToGreaterThan,

  CXX_EqualToNotEqual,
  CXX_NotEqualToEqual,

  CXX_AssignConst,
  CXX_InitConst,

  CXX_RemoveNegation,
};

std::string MutationKindToString(MutatorKind mutatorKind);

} // namespace mull

namespace std {

template <> struct hash<mull::MutatorKind> {
  std::size_t operator()(const mull::MutatorKind &k) const {
    return static_cast<std::size_t>(k);
  }
};

} // namespace std

namespace mull {
class MutatorKindSet {
public:
  static MutatorKindSet create(std::vector<MutatorKind> mutators);
  bool includesMutator(mull::MutatorKind mutatorKind) const;
private:
  MutatorKindSet(std::unordered_set<mull::MutatorKind> mutators);
  std::unordered_set<mull::MutatorKind> mutators;
};
}

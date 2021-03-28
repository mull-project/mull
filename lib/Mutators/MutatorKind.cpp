#include "mull/Mutators/MutatorKind.h"

#include <utility>

namespace mull {
std::string MutationKindToString(MutatorKind mutatorKind) {
  switch (mutatorKind) {
  default: {
    return "Unknown";
  }
  case MutatorKind::NegateMutator: {
    return "Negate";
  }
  case MutatorKind::CXX_RemoveVoidCall: {
    return "Remove Void";
  }
  case MutatorKind::CXX_ReplaceScalarCall: {
    return "Replace Call";
  }
  case MutatorKind::ScalarValueMutator: {
    return "Scalar Value";
  }
  case MutatorKind::CXX_AddToSub: {
    return "Add to Sub";
  }
  case MutatorKind::CXX_AddAssignToSubAssign: {
    return "Add-Assign to Sub-Assign";
  }
  case MutatorKind::CXX_PreIncToPreDec: {
    return "Pre-Inc to Pre-Dec";
  }
  case MutatorKind::CXX_PostIncToPostDec: {
    return "Post-Inc to Post-Dec";
  }
  case MutatorKind::CXX_SubToAdd: {
    return "Sub to Add";
  }
  case MutatorKind::CXX_SubAssignToAddAssign: {
    return "Sub-Assign to Add-Assign";
  }
  case MutatorKind::CXX_PreDecToPreInc: {
    return "Pre-Dec to Pre-Inc";
  }
  case MutatorKind::CXX_PostDecToPostInc: {
    return "Post-Dec to Post-Inc";
  }
  case MutatorKind::CXX_MulToDiv: {
    return "Mul to Div";
  }
  case MutatorKind::CXX_MulAssignToDivAssign: {
    return "Mul-Assign to Div-Assign";
  }
  case MutatorKind::CXX_DivToMul: {
    return "Div to Mul";
  }
  case MutatorKind::CXX_DivAssignToMulAssign: {
    return "Div-Assign to Mul-Assign";
  }
  case MutatorKind::CXX_RemToDiv: {
    return "Rem to Div";
  }
  case MutatorKind::CXX_RemAssignToDivAssign: {
    return "Rem-Assign to Div-Assign";
  }
  case MutatorKind::CXX_BitwiseNotToNoop: {
    return "Bitwise Not to Noop";
  }
  case MutatorKind::CXX_UnaryMinusToNoop: {
    return "Unary Minus to Noop";
  }
  case MutatorKind::CXX_LShiftToRShift: {
    return "Left Shift to Right Shift";
  }
  case MutatorKind::CXX_LShiftAssignToRShiftAssign: {
    return "Left Shift-Assign to Right Shift-Assign";
  }
  case MutatorKind::CXX_RShiftToLShift: {
    return "Right Shift to Left Shift";
  }
  case MutatorKind::CXX_RShiftAssignToLShiftAssign: {
    return "Right Shift-Assign to Left Shift-Assign";
  }
  case MutatorKind::CXX_Logical_AndToOr: {
    return "Logical And to Or";
  }
  case MutatorKind::CXX_Logical_OrToAnd: {
    return "Logical Or to And";
  }
  case MutatorKind::CXX_Bitwise_OrToAnd: {
    return "Bitwise Or to And";
  }
  case MutatorKind::CXX_Bitwise_OrAssignToAndAssign: {
    return "Bitwise Or-Assign to And-Assign";
  }
  case MutatorKind::CXX_Bitwise_AndToOr: {
    return "Bitwise And to Or";
  }
  case MutatorKind::CXX_Bitwise_AndAssignToOrAssign: {
    return "Bitwise And-Assign to Or-Assign";
  }
  case MutatorKind::CXX_Bitwise_XorToOr: {
    return "Bitwise Xor to Or";
  }
  case MutatorKind::CXX_Bitwise_XorAssignToOrAssign: {
    return "Bitwise Xor-Assign to Or-Assign";
  }
  case MutatorKind::CXX_LessThanToLessOrEqual: {
    return "Less Than to Less Or Equal";
  }
  case MutatorKind::CXX_LessOrEqualToLessThan: {
    return "Less Or Equal to Less Than";
  }
  case MutatorKind::CXX_GreaterThanToGreaterOrEqual: {
    return "Greater Than to Greater or Equal";
  }
  case MutatorKind::CXX_GreaterOrEqualToGreaterThan: {
    return "Greater Or Equal to Greater Than";
  }
  case MutatorKind::CXX_GreaterThanToLessOrEqual: {
    return "Greater Than to Less Or Equal";
  }
  case MutatorKind::CXX_GreaterOrEqualToLessThan: {
    return "Greater Or Equal to Less Than";
  }
  case MutatorKind::CXX_LessThanToGreaterOrEqual: {
    return "Less Than to Greater Or Equal";
  }
  case MutatorKind::CXX_LessOrEqualToGreaterThan: {
    return "Less Or Equal To Greater Than";
  }
  case MutatorKind::CXX_EqualToNotEqual: {
    return "Equal to Not Equal";
  }
  case MutatorKind::CXX_NotEqualToEqual: {
    return "Not Equal to Equal";
  }
  case MutatorKind::CXX_AssignConst: {
    return "Assign Const";
  }
  case MutatorKind::CXX_InitConst: {
    return "Init Const";
  }
  case MutatorKind::CXX_RemoveNegation: {
    return "Remove Unary Negation";
  }
  }
}

MutatorKindSet::MutatorKindSet(std::unordered_set<mull::MutatorKind> mutators)
    : mutators(std::move(mutators)) {}

MutatorKindSet MutatorKindSet::create(std::vector<MutatorKind> mutators) {
  std::unordered_set<MutatorKind> mutatorsSet(mutators.begin(), mutators.end());
  return MutatorKindSet(mutatorsSet);
}

bool MutatorKindSet::includesMutator(mull::MutatorKind mutatorKind) const {
  return mutators.count(mutatorKind) > 0;
}
} // namespace mull

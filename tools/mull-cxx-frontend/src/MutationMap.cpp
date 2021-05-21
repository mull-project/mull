#include "MutationMap.h"

static const std::vector<MutationIdentifier> MUTATIONS_MAP(
    { { "cxx_add_to_sub", mull::MutatorKind::CXX_AddToSub },
      { "cxx_sub_to_add", mull::MutatorKind::CXX_SubToAdd },
      { "cxx_mul_to_div", mull::MutatorKind::CXX_MulToDiv },
      { "cxx_div_to_mul", mull::MutatorKind::CXX_DivToMul },
      { "cxx_rem_to_div", mull::MutatorKind::CXX_RemToDiv },
      { "cxx_eq_to_ne", mull::MutatorKind::CXX_EqualToNotEqual },
      { "cxx_ne_to_eq", mull::MutatorKind::CXX_NotEqualToEqual },
      { "cxx_ge_to_gt", mull::MutatorKind::CXX_GreaterOrEqualToGreaterThan },
      { "cxx_ge_to_lt", mull::MutatorKind::CXX_GreaterOrEqualToLessThan },
      { "cxx_gt_to_ge", mull::MutatorKind::CXX_GreaterThanToGreaterOrEqual },
      { "cxx_gt_to_le", mull::MutatorKind::CXX_GreaterThanToLessOrEqual },
      { "cxx_le_to_gt", mull::MutatorKind::CXX_LessOrEqualToGreaterThan },
      { "cxx_le_to_lt", mull::MutatorKind::CXX_LessOrEqualToLessThan },
      { "cxx_lt_to_ge", mull::MutatorKind::CXX_LessThanToGreaterOrEqual },
      { "cxx_lt_to_le", mull::MutatorKind::CXX_LessThanToLessOrEqual },
      { "cxx_add_assign_to_sub_assign", mull::MutatorKind::CXX_AddAssignToSubAssign },
      { "cxx_sub_assign_to_add_assign", mull::MutatorKind::CXX_SubAssignToAddAssign },
      { "cxx_mul_assign_to_div_assign", mull::MutatorKind::CXX_MulAssignToDivAssign },
      { "cxx_div_assign_to_mul_assign", mull::MutatorKind::CXX_DivAssignToMulAssign },
      { "cxx_rem_assign_to_div_assign", mull::MutatorKind::CXX_RemAssignToDivAssign },
      { "cxx_and_to_or", mull::MutatorKind::CXX_Bitwise_AndToOr },
      { "cxx_or_to_and", mull::MutatorKind::CXX_Bitwise_OrToAnd },
      { "cxx_xor_to_or", mull::MutatorKind::CXX_Bitwise_XorToOr },
      { "cxx_lshift_to_rshift", mull::MutatorKind::CXX_LShiftToRShift },
      { "cxx_rshift_to_lshift", mull::MutatorKind::CXX_RShiftToLShift },
      { "cxx_and_assign_to_or_assign", mull::MutatorKind::CXX_Bitwise_AndAssignToOrAssign },
      { "cxx_or_assign_to_and_assign", mull::MutatorKind::CXX_Bitwise_OrAssignToAndAssign },
      { "cxx_xor_assign_to_or_assign", mull::MutatorKind::CXX_Bitwise_XorAssignToOrAssign },
      { "cxx_lshift_assign_to_rshift_assign", mull::MutatorKind::CXX_LShiftAssignToRShiftAssign },
      { "cxx_rshift_assign_to_lshift_assign", mull::MutatorKind::CXX_RShiftAssignToLShiftAssign },
      { "cxx_logical_and_to_or", mull::MutatorKind::CXX_Logical_AndToOr },
      { "cxx_logical_or_to_and", mull::MutatorKind::CXX_Logical_OrToAnd },
      { "cxx_remove_void_call", mull::MutatorKind::CXX_RemoveVoidCall },
      { "cxx_minus_to_noop", mull::MutatorKind::CXX_UnaryMinusToNoop },
      { "cxx_bitwise_not_to_noop", mull::MutatorKind::CXX_BitwiseNotToNoop } });

MutationMap::MutationMap() : _usedMutatorSet(), _mapKindsToIdentifiers(), _mapIdentifiersToKinds() {
  for (const MutationIdentifier &mutationIdentifier : MUTATIONS_MAP) {
    _mapKindsToIdentifiers[mutationIdentifier.mutatorKind] = mutationIdentifier.identifier;
    _mapIdentifiersToKinds[mutationIdentifier.identifier] = mutationIdentifier.mutatorKind;
  }
  assert(_mapKindsToIdentifiers.count(mull::MutatorKind::CXX_AddToSub) > 0);
}

bool MutationMap::isValidMutation(mull::MutatorKind mutatorKind) const {
  return _usedMutatorSet.count(mutatorKind) > 0;
}

std::string MutationMap::getIdentifier(mull::MutatorKind mutatorKind) {
  return _mapKindsToIdentifiers[mutatorKind];
}

void MutationMap::addMutation(std::string identifier) {
  assert(_mapIdentifiersToKinds.count(identifier) != 0);
  _usedMutatorSet.insert(_mapIdentifiersToKinds[identifier]);
}

void MutationMap::setDefaultMutationsIfNotSpecified() {
  if (!_usedMutatorSet.empty()) {
    return;
  }
  for (const MutationIdentifier &mutationIdentifier : MUTATIONS_MAP) {
    _usedMutatorSet.insert(mutationIdentifier.mutatorKind);
  }
}

#include "MutationMap.h"

static const std::vector<MutationIdentifier>
    MUTATIONS_MAP({ { "cxx_add_to_sub", mull::MutatorKind::CXX_AddToSub },
                    { "cxx_sub_to_add", mull::MutatorKind::CXX_SubToAdd },
                    { "cxx_mul_to_div", mull::MutatorKind::CXX_MulToDiv },
                    { "cxx_div_to_mul", mull::MutatorKind::CXX_DivToMul },
                    { "cxx_logical_and_to_or", mull::MutatorKind::CXX_Logical_AndToOr },
                    { "cxx_logical_or_to_and", mull::MutatorKind::CXX_Logical_OrToAnd },
                    { "cxx_remove_void_call", mull::MutatorKind::CXX_RemoveVoidCall } });

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

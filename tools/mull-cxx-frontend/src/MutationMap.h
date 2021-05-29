#pragma once

#include "mull/Mutators/MutatorKind.h"

#include <unordered_map>

struct MutationIdentifier {
  std::string identifier;
  mull::MutatorKind mutatorKind;
  MutationIdentifier(std::string identifier, mull::MutatorKind mutatorKind)
      : identifier(identifier), mutatorKind(mutatorKind) {}
};

class MutationMap {
  std::unordered_set<mull::MutatorKind> usedMutatorSet;
  std::unordered_map<mull::MutatorKind, std::string> mapKindsToIdentifiers;
  std::unordered_map<std::string, mull::MutatorKind> mapIdentifiersToKinds;

public:
  MutationMap();
  bool isValidMutation(mull::MutatorKind mutatorKind) const;
  std::string getIdentifier(mull::MutatorKind mutatorKind);
  void addMutation(std::string identifier);
  void setDefaultMutationsIfNotSpecified();
};

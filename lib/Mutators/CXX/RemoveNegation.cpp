#include "mull/Mutators/CXX/RemoveNegation.h"

using namespace mull;
using namespace mull::cxx;

static std::vector<std::unique_ptr<irm::IRMutation>> getMutators() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::NegateXORReplacement);
  return mutators;
}

std::string RemoveNegation::ID() {
  return "cxx_remove_negation";
}

RemoveNegation::RemoveNegation()
    : TrivialCXXMutator(getMutators(), MutatorKind::CXX_RemoveNegation, RemoveNegation::ID(),
                        "Replaces !a with a", "", "Replaced !a with a") {}

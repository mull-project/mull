#include "mull/Mutators/CXX/NumberMutators.h"

#include <irm/irm.h>

using namespace mull;
using namespace mull::cxx;

static std::vector<std::unique_ptr<irm::IRMutation>> getNumberMutators() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::StoreIntReplacement(42));
  mutators.emplace_back(new irm::StoreDoubleReplacement(42));
  mutators.emplace_back(new irm::StoreFloatReplacement(42));
  return mutators;
}

std::string NumberAssignConst::ID() {
  return "cxx_assign_const";
}

NumberAssignConst::NumberAssignConst()
    : TrivialCXXMutator(getNumberMutators(), MutatorKind::CXX_AssignConst, NumberAssignConst::ID(),
                        "Replaces 'a = b' with 'a = c': a constant b gets its lowest bit flipped "
                        "(sign for floats), a dynamic b becomes 42 (true for bools)",
                        "= {0};", "Replaced 'a = b' with 'a = c'") {}

std::string NumberInitConst::ID() {
  return "cxx_init_const";
}

NumberInitConst::NumberInitConst()
    : TrivialCXXMutator(getNumberMutators(), MutatorKind::CXX_InitConst, NumberInitConst::ID(),
                        "Replaces 'T a = b' with 'T a = c': a constant b gets its lowest bit "
                        "flipped (sign for floats), a dynamic b becomes 42 (true for bools)",
                        "{0}", "Replaced 'T a = b' with 'T a = c'") {}

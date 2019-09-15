#include "mull/Mutators/CXX/ArithmeticMutators.h"

using namespace mull;
using namespace mull::cxx;

/// All additive mutators share the same set of low level mutators
static std::vector<std::unique_ptr<irm::IRMutation>> getAddToSub() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::AddToSub());
  mutators.emplace_back(new irm::FAddToFSub());
  mutators.emplace_back(new irm::sadd_with_overflowTossub_with_overflow());
  return mutators;
}

const std::string AddToSub::ID = "cxx_arithmetic_add_to_sub";

AddToSub::AddToSub()
    : TrivialCXXMutator(std::move(getAddToSub()),
                        MutatorKind::CXX_Arithmetic_AddToSub, AddToSub::ID,
                        "Replaces + with -", "-", "Replaced + with -") {}

const std::string AddAssignToSubAssign::ID =
    "cxx_arithmetic_add_assign_to_sub_assign";

AddAssignToSubAssign::AddAssignToSubAssign()
    : TrivialCXXMutator(std::move(getAddToSub()),
                        MutatorKind::CXX_Arithmetic_AddAssignToSubAssign,
                        AddAssignToSubAssign::ID,
                        "Replaces += with -=", "-=", "Replaced += with -=") {}

const std::string PostIncToPostDec::ID = "cxx_arithmetic_post_inc_to_post_dec";

PostIncToPostDec::PostIncToPostDec()
    : TrivialCXXMutator(std::move(getAddToSub()),
                        MutatorKind::CXX_Arithmetic_PostIncToPostDec,
                        PostIncToPostDec::ID, "Replaces x++ with x--", "--",
                        "Replaced x++ with x--") {}

const std::string PreIncToPreDec::ID = "cxx_arithmetic_pre_inc_to_post_inc";

PreIncToPreDec::PreIncToPreDec()
    : TrivialCXXMutator(std::move(getAddToSub()),
                        MutatorKind::CXX_Arithmetic_PreIncToPreDec,
                        PreIncToPreDec::ID, "Replaces ++x with --x", "--",
                        "Replaced ++x with --x") {}
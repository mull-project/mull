#include "mull/Mutators/CXX/CallMutators.h"

using namespace mull;
using namespace mull::cxx;

static std::vector<std::unique_ptr<irm::IRMutation>> getRemoveVoidCall() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.push_back(std::make_unique<irm::RemoveVoidFunctionCall>());
  mutators.push_back(std::make_unique<irm::RemoveVoidIntrinsicsCall>());
  return mutators;
}

std::string RemoveVoidCall::ID() {
  return "cxx_remove_void_call";
}

RemoveVoidCall::RemoveVoidCall()
    : TrivialCXXMutator(getRemoveVoidCall(), MutatorKind::CXX_RemoveVoidCall, RemoveVoidCall::ID(),
                        "Removes calls to a function returning void", "",
                        "Removed the call to the function") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getReplaceScalarCall(int value) {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.push_back(std::make_unique<irm::IntCallReplacement>(value));
  mutators.push_back(std::make_unique<irm::FloatCallReplacement>(value));
  mutators.push_back(std::make_unique<irm::DoubleCallReplacement>(value));
  return mutators;
}

std::string ReplaceScalarCallZero::ID() {
  return "cxx_replace_scalar_call_zr";
}

ReplaceScalarCallZero::ReplaceScalarCallZero()
    : TrivialCXXMutator(getReplaceScalarCall(0), MutatorKind::CXX_ReplaceScalarCallZero,
                        ReplaceScalarCallZero::ID(),
                        "Replaces call to a function with 0 (false for bools)", "{0}",
                        "Replaced call to a function with zero") {}

std::string ReplaceScalarCallNonZero::ID() {
  return "cxx_replace_scalar_call_nz";
}

ReplaceScalarCallNonZero::ReplaceScalarCallNonZero()
    : TrivialCXXMutator(getReplaceScalarCall(42), MutatorKind::CXX_ReplaceScalarCallNonZero,
                        ReplaceScalarCallNonZero::ID(),
                        "Replaces call to a function with 42 (true for bools)", "{0}",
                        "Replaced call to a function with a non-zero constant") {}

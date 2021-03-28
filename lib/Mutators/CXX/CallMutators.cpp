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

static std::vector<std::unique_ptr<irm::IRMutation>> getReplaceScalarCall() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.push_back(std::make_unique<irm::IntCallReplacement>(42));
  mutators.push_back(std::make_unique<irm::FloatCallReplacement>(42));
  mutators.push_back(std::make_unique<irm::DoubleCallReplacement>(42));
  return mutators;
}

std::string ReplaceScalarCall::ID() {
  return "cxx_replace_scalar_call";
}

ReplaceScalarCall::ReplaceScalarCall()
    : TrivialCXXMutator(getReplaceScalarCall(), MutatorKind::CXX_ReplaceScalarCall,
                        ReplaceScalarCall::ID(), "Replaces call to a function with 42", "42",
                        "Replaced call to a function with 42") {}
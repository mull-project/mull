#include "mull/Mutators/CXX/BitwiseMutators.h"

using namespace mull;
using namespace mull::cxx;

static std::vector<std::unique_ptr<irm::IRMutation>> getLLShiftToLRShift() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ShlToLShr());
  return mutators;
}

std::string LShiftToRShift::ID() {
  return "cxx_lshift_to_rshift";
}

LShiftToRShift::LShiftToRShift()
    : TrivialCXXMutator(getLLShiftToLRShift(), MutatorKind::CXX_LShiftToRShift,
                        LShiftToRShift::ID(), "Replaces << with >>", ">>", "Replaced << with >>") {}

std::string LShiftAssignToRShiftAssign::ID() {
  return "cxx_lshift_assign_to_rshift_assign";
}

LShiftAssignToRShiftAssign::LShiftAssignToRShiftAssign()
    : TrivialCXXMutator(getLLShiftToLRShift(),
                        MutatorKind::CXX_LShiftAssignToRShiftAssign,
                        LShiftAssignToRShiftAssign::ID(),
                        "Replaces <<= with >>=", ">>=", "Replaced <<= with >>=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getRShiftToLShift() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::LShrToShl());
  mutators.emplace_back(new irm::AShrToShl());
  return mutators;
}

std::string RShiftToLShift::ID() {
  return "cxx_rshift_to_lshift";
}

RShiftToLShift::RShiftToLShift()
    : TrivialCXXMutator(getRShiftToLShift(), MutatorKind::CXX_RShiftToLShift,
                        RShiftToLShift::ID(), "Replaces << with >>", ">>", "Replaced >> with <<") {}

std::string RShiftAssignToLShiftAssign::ID() {
  return "cxx_rshift_assign_to_lshift_assign";
}

RShiftAssignToLShiftAssign::RShiftAssignToLShiftAssign()
    : TrivialCXXMutator(getRShiftToLShift(), MutatorKind::CXX_RShiftAssignToLShiftAssign,
                        RShiftAssignToLShiftAssign::ID(),
                        "Replaces >>= with <<=", "<<=", "Replaced >>= with <<=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getOrToAnd() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::OrToAnd());
  return mutators;
}

std::string BitwiseOrToAnd::ID() {
  return "cxx_or_to_and";
}

BitwiseOrToAnd::BitwiseOrToAnd()
    : TrivialCXXMutator(getOrToAnd(), MutatorKind::CXX_Bitwise_OrToAnd,
                        BitwiseOrToAnd::ID(), "Replaces | with &", "&", "Replaced | with &") {}

std::string OrAssignToAndAssign::ID() {
  return "cxx_or_assign_to_and_assign";
}

OrAssignToAndAssign::OrAssignToAndAssign()
    : TrivialCXXMutator(getOrToAnd(), MutatorKind::CXX_Bitwise_OrAssignToAndAssign,
                        OrAssignToAndAssign::ID(),
                        "Replaces |= with &=", "&=", "Replaced |= with &=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getAndToOr() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::AndToOr());
  return mutators;
}

std::string BitwiseAndToOr::ID() {
  return "cxx_and_to_or";
}

BitwiseAndToOr::BitwiseAndToOr()
    : TrivialCXXMutator(getAndToOr(), MutatorKind::CXX_Bitwise_AndToOr,
                        BitwiseAndToOr::ID(), "Replaces & with |", "|", "Replaced & with |") {}

std::string AndAssignToOrAssign::ID() {
  return "cxx_and_assign_to_or_assign";
}

AndAssignToOrAssign::AndAssignToOrAssign()
    : TrivialCXXMutator(getAndToOr(), MutatorKind::CXX_Bitwise_AndAssignToOrAssign,
                        AndAssignToOrAssign::ID(),
                        "Replaces &= with |=", "|=", "Replaced &= with |=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getXorToOr() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::XorToOr());
  return mutators;
}

std::string XorToOr::ID() {
  return "cxx_xor_to_or";
}

XorToOr::XorToOr()
    : TrivialCXXMutator(getXorToOr(), MutatorKind::CXX_Bitwise_XorToOr, XorToOr::ID(),
                        "Replaces ^ with |", "|", "Replaced ^ with |") {}

std::string XorAssignToOrAssign::ID() {
  return "cxx_xor_assign_to_or_assign";
}

XorAssignToOrAssign::XorAssignToOrAssign()
    : TrivialCXXMutator(getXorToOr(), MutatorKind::CXX_Bitwise_XorAssignToOrAssign,
                        XorAssignToOrAssign::ID(),
                        "Replaces ^= with |=", "|=", "Replaced ^= with |=") {}

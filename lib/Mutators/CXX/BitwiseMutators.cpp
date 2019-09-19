#include "mull/Mutators/CXX/BitwiseMutators.h"

using namespace mull;
using namespace mull::cxx;

#pragma mark - Shifts

static std::vector<std::unique_ptr<irm::IRMutation>> getLLShiftToLRShift() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ShlToLShr());
  return mutators;
}

const std::string LShiftToRShift::ID = "cxx_bitwise_lshift_to_rshift";

LShiftToRShift::LShiftToRShift()
    : TrivialCXXMutator(std::move(getLLShiftToLRShift()),
                        MutatorKind::CXX_Bitwise_LShiftToRShift,
                        LShiftToRShift::ID, "Replaces << with >>", ">>",
                        "Replaced << with >>") {}

const std::string LShiftAssignToRShiftAssign::ID =
    "cxx_bitwise_lshift_assign_to_rshift_assign";

LShiftAssignToRShiftAssign::LShiftAssignToRShiftAssign()
    : TrivialCXXMutator(std::move(getLLShiftToLRShift()),
                        MutatorKind::CXX_Bitwise_LShiftAssignToRShiftAssign,
                        LShiftAssignToRShiftAssign::ID, "Replaces <<= with >>=",
                        ">>=", "Replaced <<= with >>=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getRShiftToLShift() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::LShrToShl());
  mutators.emplace_back(new irm::AShrToShl());
  return mutators;
}

const std::string RShiftToLShift::ID = "cxx_bitwise_rshift_to_lshift";

RShiftToLShift::RShiftToLShift()
    : TrivialCXXMutator(std::move(getRShiftToLShift()),
                        MutatorKind::CXX_Bitwise_RShiftToLShift,
                        RShiftToLShift::ID, "Replaces << with >>", ">>",
                        "Replaced >> with <<") {}

const std::string RShiftAssignToLShiftAssign::ID =
    "cxx_bitwise_rshift_assign_to_lshift_assign";

RShiftAssignToLShiftAssign::RShiftAssignToLShiftAssign()
    : TrivialCXXMutator(std::move(getRShiftToLShift()),
                        MutatorKind::CXX_Bitwise_RShiftAssignToLShiftAssign,
                        RShiftAssignToLShiftAssign::ID, "Replaces >>= with <<=",
                        "<<=", "Replaced >>= with <<=") {}

#pragma mark - Bit operations

static std::vector<std::unique_ptr<irm::IRMutation>> getOrToAnd() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::OrToAnd());
  return mutators;
}

const std::string OrToAnd::ID = "cxx_bitwise_or_to_and";

OrToAnd::OrToAnd()
    : TrivialCXXMutator(std::move(getOrToAnd()),
                        MutatorKind::CXX_Bitwise_OrToAnd, OrToAnd::ID,
                        "Replaces | with &", "&", "Replaced | with &") {}

const std::string OrAssignToAndAssign::ID =
    "cxx_bitwise_or_assign_to_and_assign";

OrAssignToAndAssign::OrAssignToAndAssign()
    : TrivialCXXMutator(std::move(getOrToAnd()),
                        MutatorKind::CXX_Bitwise_OrAssignToAndAssign,
                        OrAssignToAndAssign::ID,
                        "Replaces |= with &=", "&=", "Replaced |= with &=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getAndToOr() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::AndToOr());
  return mutators;
}

const std::string AndToOr::ID = "cxx_bitwise_and_to_or";

AndToOr::AndToOr()
    : TrivialCXXMutator(std::move(getAndToOr()),
                        MutatorKind::CXX_Bitwise_AndToOr, AndToOr::ID,
                        "Replaces & with |", "|", "Replaced & with |") {}

const std::string AndAssignToOrAssign::ID =
    "cxx_bitwise_and_assign_to_or_assign";

AndAssignToOrAssign::AndAssignToOrAssign()
    : TrivialCXXMutator(std::move(getAndToOr()),
                        MutatorKind::CXX_Bitwise_AndAssignToOrAssign,
                        AndAssignToOrAssign::ID,
                        "Replaces &= with |=", "|=", "Replaced &= with |=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getXorToOr() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::XorToOr());
  return mutators;
}

const std::string XorToOr::ID = "cxx_bitwise_xor_to_or";

XorToOr::XorToOr()
    : TrivialCXXMutator(std::move(getXorToOr()),
                        MutatorKind::CXX_Bitwise_XorToOr, XorToOr::ID,
                        "Replaces ^ with |", "|", "Replaced ^ with |") {}

const std::string XorAssignToOrAssign::ID =
    "cxx_bitwise_xor_assign_to_or_assign";

XorAssignToOrAssign::XorAssignToOrAssign()
    : TrivialCXXMutator(std::move(getXorToOr()),
                        MutatorKind::CXX_Bitwise_XorAssignToOrAssign,
                        XorAssignToOrAssign::ID,
                        "Replaces ^= with |=", "|=", "Replaced ^= with |=") {}
#include "mull/Mutators/CXX/ArithmeticMutators.h"

using namespace mull;
using namespace mull::cxx;

#pragma mark - Add to sub

/// All add to sub mutators share the same set of low level mutators
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

const std::string PreIncToPreDec::ID = "cxx_arithmetic_pre_inc_to_pre_dec";

PreIncToPreDec::PreIncToPreDec()
    : TrivialCXXMutator(std::move(getAddToSub()),
                        MutatorKind::CXX_Arithmetic_PreIncToPreDec,
                        PreIncToPreDec::ID, "Replaces ++x with --x", "--",
                        "Replaced ++x with --x") {}

#pragma mark - Sub to add

static std::vector<std::unique_ptr<irm::IRMutation>> getSubToAdd() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::SubToAdd());
  mutators.emplace_back(new irm::FSubToFAdd());
  mutators.emplace_back(new irm::ssub_with_overflowTosadd_with_overflow());
  return mutators;
}

const std::string SubToAdd::ID = "cxx_arithmetic_sub_to_add";

SubToAdd::SubToAdd()
    : TrivialCXXMutator(std::move(getSubToAdd()),
                        MutatorKind::CXX_Arithmetic_SubToAdd, SubToAdd::ID,
                        "Replaces - with +", "+", "Replaced - with +") {}

const std::string SubAssignToAddAssign::ID =
    "cxx_arithmetic_sub_assign_to_add_assign";

SubAssignToAddAssign::SubAssignToAddAssign()
    : TrivialCXXMutator(std::move(getSubToAdd()),
                        MutatorKind::CXX_Arithmetic_SubAssignToAddAssign,
                        SubAssignToAddAssign::ID,
                        "Replaces -= with +=", "+=", "Replaced -= with +=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getDecToInc() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::SubToAdd());
  mutators.emplace_back(new irm::FSubToFAdd());
  mutators.emplace_back(new irm::ssub_with_overflowTosadd_with_overflow());

  /// This is somewhat non-trivial:
  /// pre and post decrements lowered to IR as
  ///     add x, -1
  /// instead of
  ///     sub x, 1
  /// So we have to consider add instructions as the one producing sub-to-add
  /// mutations
  mutators.emplace_back(new irm::AddToSub());
  return mutators;
}

const std::string PostDecToPostInc::ID = "cxx_arithmetic_post_dec_to_post_inc";

PostDecToPostInc::PostDecToPostInc()
    : TrivialCXXMutator(std::move(getDecToInc()),
                        MutatorKind::CXX_Arithmetic_PostDecToPostInc,
                        PostDecToPostInc::ID, "Replaces x-- with x++", "++",
                        "Replaced x-- with x++") {}

const std::string PreDecToPreInc::ID = "cxx_arithmetic_pre_dec_to_pre_inc";

PreDecToPreInc::PreDecToPreInc()
    : TrivialCXXMutator(std::move(getDecToInc()),
                        MutatorKind::CXX_Arithmetic_PreDecToPreInc,
                        PreDecToPreInc::ID, "Replaces --x with ++x", "++",
                        "Replaced --x with ++x") {}

#pragma mark - Mul to div

static std::vector<std::unique_ptr<irm::IRMutation>> getMulToDiv() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::MulToSDiv());
  mutators.emplace_back(new irm::FMulToFDiv());
  return mutators;
}

const std::string MulToDiv::ID = "cxx_arithmetic_mul_to_div";

MulToDiv::MulToDiv()
    : TrivialCXXMutator(std::move(getMulToDiv()),
                        MutatorKind::CXX_Arithmetic_MulToDiv, MulToDiv::ID,
                        "Replaces * with /", "/", "Replaced * with /") {}

const std::string MulAssignToDivAssign::ID =
    "cxx_arithmetic_mul_assign_to_div_assign";

MulAssignToDivAssign::MulAssignToDivAssign()
    : TrivialCXXMutator(std::move(getMulToDiv()),
                        MutatorKind::CXX_Arithmetic_MulAssignToDivAssign,
                        MulAssignToDivAssign::ID,
                        "Replaces *= with /=", "/=", "Replaced *= with /=") {}

#pragma mark - Div to mul

static std::vector<std::unique_ptr<irm::IRMutation>> getDivToMul() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::FDivToFMul());
  mutators.emplace_back(new irm::SDivToMul());
  mutators.emplace_back(new irm::UDivToMul());
  return mutators;
}

const std::string DivToMul::ID = "cxx_arithmetic_div_to_mul";

DivToMul::DivToMul()
    : TrivialCXXMutator(std::move(getDivToMul()),
                        MutatorKind::CXX_Arithmetic_DivToMul, DivToMul::ID,
                        "Replaces / with *", "*", "Replaced / with *") {}

const std::string DivAssignToMulAssign::ID =
    "cxx_arithmetic_div_assign_to_mul_assign";

DivAssignToMulAssign::DivAssignToMulAssign()
    : TrivialCXXMutator(std::move(getDivToMul()),
                        MutatorKind::CXX_Arithmetic_DivAssignToMulAssign,
                        DivAssignToMulAssign::ID,
                        "Replaces /= with *=", "*=", "Replaced /= with *=") {}

#pragma mark - Rem to div

static std::vector<std::unique_ptr<irm::IRMutation>> getRemToDiv() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::FRemToFDiv());
  mutators.emplace_back(new irm::SRemToSDiv());
  mutators.emplace_back(new irm::URemToUDiv());
  return mutators;
}

const std::string RemToDiv::ID = "cxx_arithmetic_rem_to_div";

RemToDiv::RemToDiv()
    : TrivialCXXMutator(std::move(getRemToDiv()),
                        MutatorKind::CXX_Arithmetic_RemToDiv, RemToDiv::ID,
                        "Replaces % with /", "/", "Replaced % with /") {}

const std::string RemAssignToDivAssign::ID =
    "cxx_arithmetic_rem_assign_to_div_assign";

RemAssignToDivAssign::RemAssignToDivAssign()
    : TrivialCXXMutator(std::move(getRemToDiv()),
                        MutatorKind::CXX_Arithmetic_RemAssignToDivAssign,
                        RemAssignToDivAssign::ID,
                        "Replaces %= with /=", "/=", "Replaced %= with /=") {}

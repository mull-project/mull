#include "mull/Mutators/CXX/RelationalMutators.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace mull;
using namespace mull::cxx;

static std::vector<std::unique_ptr<irm::IRMutation>> getLessThanToLessOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLTToICMP_SLE());
  mutators.emplace_back(new irm::ICMP_ULTToICMP_ULE());
  mutators.emplace_back(new irm::FCMP_OLTToFCMP_OLE());
  mutators.emplace_back(new irm::FCMP_ULTToFCMP_ULE());
  return mutators;
}

std::string LessThanToLessOrEqual::ID() {
  return "cxx_lt_to_le";
}

LessThanToLessOrEqual::LessThanToLessOrEqual()
    : TrivialCXXMutator(getLessThanToLessOrEqual(), MutatorKind::CXX_LessThanToLessOrEqual,
                        LessThanToLessOrEqual::ID(),
                        "Replaces < with <=", "<=", "Replaced < with <=") {}

std::string LessOrEqualToLessThan::ID() {
  return "cxx_le_to_lt";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getLessOrEqualToLessThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLEToICMP_SLT());
  mutators.emplace_back(new irm::ICMP_ULEToICMP_ULT());
  mutators.emplace_back(new irm::FCMP_OLEToFCMP_OLT());
  mutators.emplace_back(new irm::FCMP_ULEToFCMP_ULT());
  return mutators;
}

LessOrEqualToLessThan::LessOrEqualToLessThan()
    : TrivialCXXMutator(getLessOrEqualToLessThan(), MutatorKind::CXX_LessOrEqualToLessThan,
                        LessOrEqualToLessThan::ID(), "Replaces <= with <", "<",
                        "Replaced <= with <") {}

std::string GreaterThanToGreaterOrEqual::ID() {
  return "cxx_gt_to_ge";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getGreaterThanToGreaterOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGTToICMP_SGE());
  mutators.emplace_back(new irm::ICMP_UGTToICMP_UGE());
  mutators.emplace_back(new irm::FCMP_OGTToFCMP_OGE());
  mutators.emplace_back(new irm::FCMP_UGTToFCMP_UGE());
  return mutators;
}

GreaterThanToGreaterOrEqual::GreaterThanToGreaterOrEqual()
    : TrivialCXXMutator(
          getGreaterThanToGreaterOrEqual(), MutatorKind::CXX_GreaterThanToGreaterOrEqual,
          GreaterThanToGreaterOrEqual::ID(), "Replaces > with >=", ">=", "Replaced > with >=") {}

std::string GreaterOrEqualToGreaterThan::ID() {
  return "cxx_ge_to_gt";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getGreaterOrEqualToGreaterThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGEToICMP_SGT());
  mutators.emplace_back(new irm::ICMP_UGEToICMP_UGT());
  mutators.emplace_back(new irm::FCMP_OGEToFCMP_OGT());
  mutators.emplace_back(new irm::FCMP_UGEToFCMP_UGT());
  return mutators;
}

GreaterOrEqualToGreaterThan::GreaterOrEqualToGreaterThan()
    : TrivialCXXMutator(
          getGreaterOrEqualToGreaterThan(), MutatorKind::CXX_GreaterOrEqualToGreaterThan,
          GreaterOrEqualToGreaterThan::ID(), "Replaces >= with >", ">", "Replaced >= with >") {}

std::string EqualToNotEqual::ID() {
  return "cxx_eq_to_ne";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getEqualToNotEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_EQToICMP_NE());
  mutators.emplace_back(new irm::FCMP_OEQToFCMP_ONE());
  mutators.emplace_back(new irm::FCMP_UEQToFCMP_UNE());
  return mutators;
}

EqualToNotEqual::EqualToNotEqual()
    : TrivialCXXMutator(getEqualToNotEqual(), MutatorKind::CXX_EqualToNotEqual,
                        EqualToNotEqual::ID(), "Replaces == with !=", "!=", "Replaced == with !=") {
}

std::string NotEqualToEqual::ID() {
  return "cxx_ne_to_eq";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getNotEqualToEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_NEToICMP_EQ());
  mutators.emplace_back(new irm::FCMP_ONEToFCMP_OEQ());
  mutators.emplace_back(new irm::FCMP_UNEToFCMP_UEQ());
  return mutators;
}

NotEqualToEqual::NotEqualToEqual()
    : TrivialCXXMutator(getNotEqualToEqual(), MutatorKind::CXX_NotEqualToEqual,
                        NotEqualToEqual::ID(), "Replaces != with ==", "==", "Replaced != with ==") {
}

std::string GreaterThanToLessOrEqual::ID() {
  return "cxx_gt_to_le";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getGreaterThanToLessOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGTToICMP_SLE());
  mutators.emplace_back(new irm::ICMP_UGTToICMP_ULE());
  mutators.emplace_back(new irm::FCMP_OGTToFCMP_OLE());
  mutators.emplace_back(new irm::FCMP_UGTToFCMP_ULE());
  return mutators;
}

GreaterThanToLessOrEqual::GreaterThanToLessOrEqual()
    : TrivialCXXMutator(getGreaterThanToLessOrEqual(), MutatorKind::CXX_GreaterThanToLessOrEqual,
                        GreaterThanToLessOrEqual::ID(),
                        "Replaces > with <=", "<=", "Replaced > with <=") {}

std::string GreaterOrEqualToLessThan::ID() {
  return "cxx_ge_to_lt";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getGreaterOrEqualToLessThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGEToICMP_SLT());
  mutators.emplace_back(new irm::ICMP_UGEToICMP_ULT());
  mutators.emplace_back(new irm::FCMP_OGEToFCMP_OLT());
  mutators.emplace_back(new irm::FCMP_UGEToFCMP_ULT());
  return mutators;
}

GreaterOrEqualToLessThan::GreaterOrEqualToLessThan()
    : TrivialCXXMutator(getGreaterOrEqualToLessThan(), MutatorKind::CXX_GreaterOrEqualToLessThan,
                        GreaterOrEqualToLessThan::ID(), "Replaces >= with <", "<",
                        "Replaced >= with <") {}

std::string LessThanToGreaterOrEqual::ID() {
  return "cxx_lt_to_ge";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getLessThanToGreaterOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLTToICMP_SGE());
  mutators.emplace_back(new irm::ICMP_ULTToICMP_UGE());
  mutators.emplace_back(new irm::FCMP_OLTToFCMP_OGE());
  mutators.emplace_back(new irm::FCMP_ULTToFCMP_UGE());
  return mutators;
}

LessThanToGreaterOrEqual::LessThanToGreaterOrEqual()
    : TrivialCXXMutator(getLessThanToGreaterOrEqual(), MutatorKind::CXX_LessThanToGreaterOrEqual,
                        LessThanToGreaterOrEqual::ID(),
                        "Replaces < with >=", ">=", "Replaced < with >=") {}

std::string LessOrEqualToGreaterThan::ID() {
  return "cxx_le_to_gt";
}

static std::vector<std::unique_ptr<irm::IRMutation>> getLessOrEqualToGreaterThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLEToICMP_SGT());
  mutators.emplace_back(new irm::ICMP_ULEToICMP_UGT());
  mutators.emplace_back(new irm::FCMP_OLEToFCMP_OGT());
  mutators.emplace_back(new irm::FCMP_ULEToFCMP_UGT());
  return mutators;
}

LessOrEqualToGreaterThan::LessOrEqualToGreaterThan()
    : TrivialCXXMutator(getLessOrEqualToGreaterThan(), MutatorKind::CXX_LessOrEqualToGreaterThan,
                        LessOrEqualToGreaterThan::ID(), "Replaces <= with >", ">",
                        "Replaced <= with >") {}

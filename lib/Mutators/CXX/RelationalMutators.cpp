#include "mull/Mutators/CXX/RelationalMutators.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace mull;
using namespace mull::cxx;

const std::string LessThanToLessOrEqual::ID = "cxx_relational_lt_to_le";

static std::vector<std::unique_ptr<irm::IRMutation>>
getLessThanToLessOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLTToICMP_SLE());
  mutators.emplace_back(new irm::ICMP_ULTToICMP_ULE());
  mutators.emplace_back(new irm::FCMP_OLTToFCMP_OLE());
  mutators.emplace_back(new irm::FCMP_ULTToFCMP_ULE());
  return mutators;
}

LessThanToLessOrEqual::LessThanToLessOrEqual()
    : TrivialCXXMutator(std::move(getLessThanToLessOrEqual()),
                        MutatorKind::CXX_Relation_LessThanToLessOrEqual,
                        LessThanToLessOrEqual::ID,
                        "Replaces < with <=", "<=", "Replaced < with <=") {}

const std::string LessOrEqualToLessThan::ID = "cxx_relational_le_to_lt";

static std::vector<std::unique_ptr<irm::IRMutation>>
getLessOrEqualToLessThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLEToICMP_SLT());
  mutators.emplace_back(new irm::ICMP_ULEToICMP_ULT());
  mutators.emplace_back(new irm::FCMP_OLEToFCMP_OLT());
  mutators.emplace_back(new irm::FCMP_ULEToFCMP_ULT());
  return mutators;
}

LessOrEqualToLessThan::LessOrEqualToLessThan()
    : TrivialCXXMutator(std::move(getLessOrEqualToLessThan()),
                        MutatorKind::CXX_Relation_LessOrEqualToLessThan,
                        LessOrEqualToLessThan::ID, "Replaces <= with <", "<",
                        "Replaced <= with <") {}

const std::string GreaterThanToGreaterOrEqual::ID = "cxx_relational_gt_to_ge";

static std::vector<std::unique_ptr<irm::IRMutation>>
getGreaterThanToGreaterOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGTToICMP_SGE());
  mutators.emplace_back(new irm::ICMP_UGTToICMP_UGE());
  mutators.emplace_back(new irm::FCMP_OGTToFCMP_OGE());
  mutators.emplace_back(new irm::FCMP_UGTToFCMP_UGE());
  return mutators;
}

GreaterThanToGreaterOrEqual::GreaterThanToGreaterOrEqual()
    : TrivialCXXMutator(std::move(getGreaterThanToGreaterOrEqual()),
                        MutatorKind::CXX_Relation_GreaterThanToGreaterOrEqual,
                        GreaterThanToGreaterOrEqual::ID,
                        "Replaces > with >=", ">=", "Replaced > with >=") {}

const std::string GreaterOrEqualToGreaterThan::ID = "cxx_relational_ge_to_gt";

static std::vector<std::unique_ptr<irm::IRMutation>>
getGreaterOrEqualToGreaterThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGEToICMP_SGT());
  mutators.emplace_back(new irm::ICMP_UGEToICMP_UGT());
  mutators.emplace_back(new irm::FCMP_OGEToFCMP_OGT());
  mutators.emplace_back(new irm::FCMP_UGEToFCMP_UGT());
  return mutators;
}

GreaterOrEqualToGreaterThan::GreaterOrEqualToGreaterThan()
    : TrivialCXXMutator(std::move(getGreaterOrEqualToGreaterThan()),
                        MutatorKind::CXX_Relation_GreaterOrEqualToGreaterThan,
                        GreaterOrEqualToGreaterThan::ID, "Replaces >= with >",
                        ">", "Replaced >= with >") {}

const std::string EqualToNotEqual::ID = "cxx_relational_eq_to_ne";

static std::vector<std::unique_ptr<irm::IRMutation>> getEqualToNotEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_EQToICMP_NE());
  mutators.emplace_back(new irm::FCMP_OEQToFCMP_ONE());
  mutators.emplace_back(new irm::FCMP_UEQToFCMP_UNE());
  return mutators;
}

EqualToNotEqual::EqualToNotEqual()
    : TrivialCXXMutator(std::move(getEqualToNotEqual()),
                        MutatorKind::CXX_Relation_EqualToNotEqual,
                        EqualToNotEqual::ID,
                        "Replaces == with !=", "!=", "Replaced != with !=") {}

const std::string NotEqualToEqual::ID = "cxx_relational_ne_to_eq";

static std::vector<std::unique_ptr<irm::IRMutation>> getNotEqualToEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_NEToICMP_EQ());
  mutators.emplace_back(new irm::FCMP_ONEToFCMP_OEQ());
  mutators.emplace_back(new irm::FCMP_UNEToFCMP_UEQ());
  return mutators;
}

NotEqualToEqual::NotEqualToEqual()
    : TrivialCXXMutator(std::move(getNotEqualToEqual()),
                        MutatorKind::CXX_Relation_NotEqualToEqual,
                        NotEqualToEqual::ID,
                        "Replaces != with ==", "==", "Replaced != with ==") {}

const std::string GreaterThanToLessOrEqual::ID = "cxx_relational_gt_to_le";

static std::vector<std::unique_ptr<irm::IRMutation>>
getGreaterThanToLessOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGTToICMP_SLE());
  mutators.emplace_back(new irm::ICMP_UGTToICMP_ULE());
  mutators.emplace_back(new irm::FCMP_OGTToFCMP_OLE());
  mutators.emplace_back(new irm::FCMP_UGTToFCMP_ULE());
  return mutators;
}

GreaterThanToLessOrEqual::GreaterThanToLessOrEqual()
    : TrivialCXXMutator(std::move(getGreaterThanToLessOrEqual()),
                        MutatorKind::CXX_Relation_GreaterThanToLessOrEqual,
                        GreaterThanToLessOrEqual::ID,
                        "Replaces > with <=", "<=", "Replaced > with <=") {}

const std::string GreaterOrEqualToLessThan::ID = "cxx_relational_ge_to_lt";

static std::vector<std::unique_ptr<irm::IRMutation>>
getGreaterOrEqualToLessThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SGEToICMP_SLT());
  mutators.emplace_back(new irm::ICMP_UGEToICMP_ULT());
  mutators.emplace_back(new irm::FCMP_OGEToFCMP_OLT());
  mutators.emplace_back(new irm::FCMP_UGEToFCMP_ULT());
  return mutators;
}

GreaterOrEqualToLessThan::GreaterOrEqualToLessThan()
    : TrivialCXXMutator(std::move(getGreaterOrEqualToLessThan()),
                        MutatorKind::CXX_Relation_GreaterOrEqualToLessThan,
                        GreaterOrEqualToLessThan::ID, "Replaces >= with <", "<",
                        "Replaced >= with <") {}

const std::string LessThanToGreaterOrEqual::ID = "cxx_relational_lt_to_ge";

static std::vector<std::unique_ptr<irm::IRMutation>>
getLessThanToGreaterOrEqual() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLTToICMP_SGE());
  mutators.emplace_back(new irm::ICMP_ULTToICMP_UGE());
  mutators.emplace_back(new irm::FCMP_OLTToFCMP_OGE());
  mutators.emplace_back(new irm::FCMP_ULTToFCMP_UGE());
  return mutators;
}

LessThanToGreaterOrEqual::LessThanToGreaterOrEqual()
    : TrivialCXXMutator(std::move(getLessThanToGreaterOrEqual()),
                        MutatorKind::CXX_Relation_LessThanToGreaterOrEqual,
                        LessThanToGreaterOrEqual::ID,
                        "Replaces < with >=", ">=", "Replaced < with >=") {}

const std::string LessOrEqualToGreaterThan::ID = "cxx_relational_le_to_gt";

static std::vector<std::unique_ptr<irm::IRMutation>>
getLessOrEqualToGreaterThan() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::ICMP_SLEToICMP_SGT());
  mutators.emplace_back(new irm::ICMP_ULEToICMP_UGT());
  mutators.emplace_back(new irm::FCMP_OLEToFCMP_OGT());
  mutators.emplace_back(new irm::FCMP_ULEToFCMP_UGT());
  return mutators;
}

LessOrEqualToGreaterThan::LessOrEqualToGreaterThan()
    : TrivialCXXMutator(std::move(getLessOrEqualToGreaterThan()),
                        MutatorKind::CXX_Relation_LessOrEqualToGreaterThan,
                        LessOrEqualToGreaterThan::ID, "Replaces <= with >", ">",
                        "Replaced <= with >") {}

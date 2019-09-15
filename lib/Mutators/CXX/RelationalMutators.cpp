#include "mull/Mutators/CXX/RelationalMutators.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>

using namespace mull;
using namespace mull::cxx;

const std::string LessThanToLessOrEqual::ID = "cxx_relational_lt_to_le";

std::vector<std::unique_ptr<irm::IRMutation>> getLessThanToLessOrEqual() {
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

std::vector<std::unique_ptr<irm::IRMutation>> getLessOrEqualToLessThan() {
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

std::vector<std::unique_ptr<irm::IRMutation>> getGreaterThanToGreaterOrEqual() {
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

std::vector<std::unique_ptr<irm::IRMutation>> getGreaterOrEqualToGreaterThan() {
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
                        GreaterOrEqualToGreaterThan::ID,
                        "Replaces >= with >", ">", "Replaced >= with >") {}

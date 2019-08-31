#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string NegateConditionMutator::ID = "negate_mutator";
const std::string NegateConditionMutator::description =
    "Negates conditionals, e.g.: != to ==, > to <=";

NegateConditionMutator::NegateConditionMutator() : lowLevelMutators() {
  /// == -> !=
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_EQToICMP_NE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OEQToFCMP_ONE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UEQToFCMP_UNE>());
  /// != -> ==
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_NEToICMP_EQ>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_ONEToFCMP_OEQ>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UNEToFCMP_UEQ>());
  /// >  -> <=
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SGTToICMP_SLE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_UGTToICMP_ULE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OGTToFCMP_OLE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UGTToFCMP_ULE>());
  /// >= -> <
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SGEToICMP_SLT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_UGEToICMP_ULT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OGEToFCMP_OLT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UGEToFCMP_ULT>());
  /// <  -> >=
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SLTToICMP_SGE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_ULTToICMP_UGE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OLTToFCMP_OGE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_ULTToFCMP_UGE>());
  /// <= -> >
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SLEToICMP_SGT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_ULEToICMP_UGT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OLEToFCMP_OGT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_ULEToFCMP_UGT>());

  /// ??
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_TRUEToFCMP_FALSE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_FALSEToFCMP_TRUE>());
}

static std::string describePredicate(CmpInst::Predicate predicate) {
  switch (predicate) {

  case CmpInst::FCMP_FALSE:
    return "Always false";

  case CmpInst::FCMP_OEQ:
    return "==";

  case CmpInst::FCMP_OGT:
    return ">";

  case CmpInst::FCMP_OGE:
    return ">=";

  case CmpInst::FCMP_OLT:
    return "<";

  case CmpInst::FCMP_OLE:
    return "<=";

  case CmpInst::FCMP_ONE:
    return "!=";

  case CmpInst::FCMP_ORD:
    return "TODO: Ordered (no nans)";

  case CmpInst::FCMP_UNO:
    return "TODO: Unordered (isnan(X) | isnan(Y))";

  case CmpInst::FCMP_UEQ:
    return "=";

  case CmpInst::FCMP_UGT:
    return ">";

  case CmpInst::FCMP_UGE:
    return ">=";

  case CmpInst::FCMP_ULT:
    return "<";

  case CmpInst::FCMP_ULE:
    return "<=";

  case CmpInst::FCMP_UNE:
    return "!=";

  case CmpInst::FCMP_TRUE:
    return "TODO: Always True";

  case CmpInst::ICMP_EQ:
    return "==";

  case CmpInst::ICMP_NE:
    return "!=";

  case CmpInst::ICMP_UGT:
    return ">";

  case CmpInst::ICMP_UGE:
    return ">=";

  case CmpInst::ICMP_ULT:
    return "<";

  case CmpInst::ICMP_ULE:
    return "<=";

  case CmpInst::ICMP_SGT:
    return ">";

  case CmpInst::ICMP_SGE:
    return ">=";

  case CmpInst::ICMP_SLT:
    return "<";

  case CmpInst::ICMP_SLE:
    return "<=";

  default:
    return "TODO";
  }
}

static std::string getDiagnostics(CmpInst::Predicate originalPredicate,
                                  CmpInst::Predicate negatedPredicate) {
  std::stringstream diagnostics;
  diagnostics << "Negate Condition: replaced ";
  diagnostics << describePredicate(originalPredicate);
  diagnostics << " with ";
  diagnostics << describePredicate(negatedPredicate);

  return diagnostics.str();
}

void NegateConditionMutator::applyMutation(llvm::Function *function,
                                           const MutationPointAddress &address,
                                           irm::IRMutation *lowLevelMutation) {

  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
NegateConditionMutator::getMutations(Bitcode *bitcode,
                                     llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    for (auto &llMutation : lowLevelMutators) {
      if (llMutation->canMutate(&instruction)) {

        auto cmpMutator =
            reinterpret_cast<irm::_CmpInstPredicateReplacementBase *>(
                llMutation.get());

        std::string diagnostics =
            getDiagnostics(cmpMutator->_getFrom(), cmpMutator->_getTo());

        std::string replacement = describePredicate(cmpMutator->_getTo());

        auto point = new MutationPoint(this, llMutation.get(), &instruction,
                                       replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

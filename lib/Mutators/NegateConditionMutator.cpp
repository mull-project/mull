#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/MutationPoint.h"
#include "mull/ReachableFunction.h"
#include <irm/irm.h>
#include <sstream>

using namespace llvm;
using namespace mull;

std::string NegateConditionMutator::ID() {
  return "negate_mutator";
}
std::string NegateConditionMutator::description() {
  return "Negates conditionals !x to x and x to !x";
}

NegateConditionMutator::NegateConditionMutator() : lowLevelMutators(), negateXorMutator() {
  /// == -> !=
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_EQToICMP_NE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OEQToFCMP_ONE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UEQToFCMP_UNE>());
  /// != -> ==
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_NEToICMP_EQ>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_ONEToFCMP_OEQ>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UNEToFCMP_UEQ>());
}

static std::string describePredicate(CmpInst::Predicate predicate) {
  switch (predicate) {

  case CmpInst::FCMP_OEQ:
  case CmpInst::FCMP_UEQ:
  case CmpInst::ICMP_EQ:
    return "==";

  case CmpInst::FCMP_ONE:
  case CmpInst::FCMP_UNE:
  case CmpInst::ICMP_NE:
    return "!=";

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
NegateConditionMutator::getMutations(Bitcode *bitcode, const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    if (negateXorMutator.canMutate(instruction)) {
      std::string diagnostics = "Negate Condition: replaced !x with !!x";
      auto point =
        new MutationPoint(this, &negateXorMutator, instruction, "", bitcode, diagnostics);
      mutations.push_back(point);
    }

    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {

        auto cmpMutator = reinterpret_cast<irm::_CmpInstPredicateReplacementBase *>(mutator.get());

        std::string diagnostics = getDiagnostics(cmpMutator->_getFrom(), cmpMutator->_getTo());

        std::string replacement = describePredicate(cmpMutator->_getTo());

        auto point =
            new MutationPoint(this, mutator.get(), instruction, replacement, bitcode, diagnostics);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

#include "mull/Mutators/NegateConditionMutator.h"
#include "mull/FunctionUnderTest.h"
#include "mull/MutationPoint.h"
#include <cassert>
#include <irm/irm.h>

using namespace llvm;
using namespace mull;

std::string NegateConditionMutator::ID() {
  return "negate_mutator";
}
std::string NegateConditionMutator::description() {
  return "Negates conditionals !x to x and x to !x";
}

NegateConditionMutator::NegateConditionMutator() : lowLevelMutators() {
  /// == -> !=
  lowLevelMutators.push_back(std::make_unique<irm::ICMP_EQToICMP_NE>());
  lowLevelMutators.push_back(std::make_unique<irm::FCMP_OEQToFCMP_ONE>());
  lowLevelMutators.push_back(std::make_unique<irm::FCMP_UEQToFCMP_UNE>());
  /// != -> ==
  lowLevelMutators.push_back(std::make_unique<irm::ICMP_NEToICMP_EQ>());
  lowLevelMutators.push_back(std::make_unique<irm::FCMP_ONEToFCMP_OEQ>());
  lowLevelMutators.push_back(std::make_unique<irm::FCMP_UNEToFCMP_UEQ>());
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

    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {

        auto cmpMutator = reinterpret_cast<irm::_CmpInstPredicateReplacementBase *>(mutator.get());

        std::string replacement = describePredicate(cmpMutator->_getTo());

        auto point = new MutationPoint(this, mutator.get(), instruction, replacement, bitcode);
        mutations.push_back(point);
      }
    }
  }

  return mutations;
}

#include "mull/Mutators/ConditionalsBoundaryMutator.h"
#include "mull/MutationPoint.h"
#include <irm/irm.h>
#include <llvm/IR/InstIterator.h>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string ConditionalsBoundaryMutator::ID =
    "conditionals_boundary_mutator";
const std::string ConditionalsBoundaryMutator::description =
    "Replaces > with >=, >= with >, < with <=, <= with <";

///
/// This mutator changes boundaries of conditionals:
///
///   <   |  <=
///   <=  |  <
///   >   |  >=
///   >=  |  >
///
/// We apply it whenever possible and let the junk detector
/// filter out irrelevant mutations
///

ConditionalsBoundaryMutator::ConditionalsBoundaryMutator()
    : lowLevelMutators() {
  ///  >  | >=
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SGTToICMP_SGE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_UGTToICMP_UGE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OGTToFCMP_OGE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UGTToFCMP_UGE>());

  ///  >= | >
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SGEToICMP_SGT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_UGEToICMP_UGT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OGEToFCMP_OGT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_UGEToFCMP_UGT>());

  ///  <  | <=
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SLTToICMP_SLE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_ULTToICMP_ULE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OLTToFCMP_OLE>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_ULTToFCMP_ULE>());

  ///  <= | <
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_SLEToICMP_SLT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::ICMP_ULEToICMP_ULT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_OLEToFCMP_OLT>());
  lowLevelMutators.push_back(llvm::make_unique<irm::FCMP_ULEToFCMP_ULT>());
}

static std::string describePredicate(CmpInst::Predicate predicate) {
  switch (predicate) {

  case CmpInst::ICMP_SGT:
  case CmpInst::ICMP_UGT:
  case CmpInst::FCMP_UGT:
  case CmpInst::FCMP_OGT:
    return ">";

  case CmpInst::ICMP_SGE:
  case CmpInst::ICMP_UGE:
  case CmpInst::FCMP_UGE:
  case CmpInst::FCMP_OGE:
    return ">=";

  case CmpInst::ICMP_SLT:
  case CmpInst::ICMP_ULT:
  case CmpInst::FCMP_ULT:
  case CmpInst::FCMP_OLT:
    return "<";

  case CmpInst::ICMP_SLE:
  case CmpInst::ICMP_ULE:
  case CmpInst::FCMP_ULE:
  case CmpInst::FCMP_OLE:
    return "<=";

  default:
    return "unsupported?";
  }
}

static std::string getDiagnostics(CmpInst::Predicate originalPredicate,
                                  CmpInst::Predicate negatedPredicate) {
  std::stringstream diagnostics;
  diagnostics << "Boundary Mutation: replaced ";
  diagnostics << describePredicate(originalPredicate);
  diagnostics << " with ";
  diagnostics << describePredicate(negatedPredicate);
  diagnostics << "\n";

  return diagnostics.str();
}

std::string ConditionalsBoundaryMutator::getUniqueIdentifier() { return ID; }

std::string ConditionalsBoundaryMutator::getUniqueIdentifier() const {
  return ID;
}

std::string ConditionalsBoundaryMutator::getDescription() const {
  return description;
}

MutatorKind ConditionalsBoundaryMutator::mutatorKind() {
  return MutatorKind::ConditionalsBoundaryMutator;
}

void ConditionalsBoundaryMutator::applyMutation(
    llvm::Function *function, const MutationPointAddress &address,
    irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

std::vector<MutationPoint *>
ConditionalsBoundaryMutator::getMutations(Bitcode *bitcode,
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

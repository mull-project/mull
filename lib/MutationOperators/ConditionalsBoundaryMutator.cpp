#include "MutationOperators/ConditionalsBoundaryMutator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

const std::string ConditionalsBoundaryMutator::ID = "conditionals_boundary_mutator";

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

static llvm::Optional<CmpInst::Predicate>
getMutatedPredicate(CmpInst::Predicate predicate) {
  switch (predicate) {
    ///  >  | >=
    case CmpInst::ICMP_SGT: return CmpInst::ICMP_SGE;
    case CmpInst::ICMP_UGT: return CmpInst::ICMP_UGE;
    case CmpInst::FCMP_UGT: return CmpInst::FCMP_UGE;
    case CmpInst::FCMP_OGT: return CmpInst::FCMP_OGE;

    ///  >= | >
    case CmpInst::ICMP_SGE: return CmpInst::ICMP_SGT;
    case CmpInst::ICMP_UGE: return CmpInst::ICMP_UGT;
    case CmpInst::FCMP_UGE: return CmpInst::FCMP_UGT;
    case CmpInst::FCMP_OGE: return CmpInst::FCMP_OGT;

    ///  <  | <=
    case CmpInst::ICMP_SLT: return CmpInst::ICMP_SLE;
    case CmpInst::ICMP_ULT: return CmpInst::ICMP_ULE;
    case CmpInst::FCMP_ULT: return CmpInst::FCMP_ULE;
    case CmpInst::FCMP_OLT: return CmpInst::FCMP_OLE;

    ///  <= | <
    case CmpInst::ICMP_SLE: return CmpInst::ICMP_SLT;
    case CmpInst::ICMP_ULE: return CmpInst::ICMP_ULT;
    case CmpInst::FCMP_ULE: return CmpInst::FCMP_ULT;
    case CmpInst::FCMP_OLE: return CmpInst::FCMP_OLT;

    default:
      return None;
      break;
  }
  return None;
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

bool ConditionalsBoundaryMutator::isGT(llvm::Instruction *instruction) {
  CmpInst *cmp = dyn_cast<CmpInst>(instruction);
  if (cmp == nullptr) {
    return false;
  }

  CmpInst::Predicate predicate = cmp->getPredicate();
  switch (predicate) {
    case CmpInst::ICMP_SGT:
    case CmpInst::ICMP_UGT:
    case CmpInst::FCMP_UGT:
    case CmpInst::FCMP_OGT:
      return true;
      break;

    default:
      return false;
      break;
  }

  return false;
}

bool ConditionalsBoundaryMutator::isGTE(llvm::Instruction *instruction) {
  CmpInst *cmp = dyn_cast<CmpInst>(instruction);
  if (cmp == nullptr) {
    return false;
  }

  CmpInst::Predicate predicate = cmp->getPredicate();
  switch (predicate) {
    case CmpInst::ICMP_SGE:
    case CmpInst::ICMP_UGE:
    case CmpInst::FCMP_UGE:
    case CmpInst::FCMP_OGE:
      return true;
      break;

    default:
      return false;
      break;
  }

  return false;
}

bool ConditionalsBoundaryMutator::isLT(llvm::Instruction *instruction) {
  CmpInst *cmp = dyn_cast<CmpInst>(instruction);
  if (cmp == nullptr) {
    return false;
  }

  CmpInst::Predicate predicate = cmp->getPredicate();
  switch (predicate) {
    case CmpInst::ICMP_SLT:
    case CmpInst::ICMP_ULT:
    case CmpInst::FCMP_ULT:
    case CmpInst::FCMP_OLT:
      return true;
      break;

    default:
      return false;
      break;
  }

  return false;
}

bool ConditionalsBoundaryMutator::isLTE(llvm::Instruction *instruction) {
  CmpInst *cmp = dyn_cast<CmpInst>(instruction);
  if (cmp == nullptr) {
    return false;
  }

  CmpInst::Predicate predicate = cmp->getPredicate();
  switch (predicate) {
    case CmpInst::ICMP_SLE:
    case CmpInst::ICMP_ULE:
    case CmpInst::FCMP_ULE:
    case CmpInst::FCMP_OLE:
      return true;
      break;

    default:
      return false;
      break;
  }

  return false;
}

std::string ConditionalsBoundaryMutator::uniqueID() {
  return ID;
}

std::string ConditionalsBoundaryMutator::uniqueID() const {
  return ID;
}

MutatorKind ConditionalsBoundaryMutator::mutatorKind() {
  return MutatorKind::ConditionalsBoundaryMutator;
}

MutationPoint *
ConditionalsBoundaryMutator::getMutationPoint(MullModule *module,
                                              MutationPointAddress &address,
                                              Instruction *instruction) {
  CmpInst *cmpOp = dyn_cast<CmpInst>(instruction);
  if (cmpOp == nullptr) {
    return nullptr;
  }

  CmpInst::Predicate originalPredicate = cmpOp->getPredicate();
  Optional<CmpInst::Predicate> mutatedPredicate = getMutatedPredicate(originalPredicate);
  if (!mutatedPredicate.hasValue()) {
    return nullptr;
  }

  std::string diagnostics = getDiagnostics(originalPredicate,
                                           mutatedPredicate.getValue());

  return new MutationPoint(this, address, instruction, module, diagnostics);
}

bool ConditionalsBoundaryMutator::canBeApplied(Value &V) {
  llvm_unreachable("not used here anymore");
  return false;
}

Value *ConditionalsBoundaryMutator::applyMutation(Module *M, MutationPointAddress address, Value &_V) {
  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module

  Function &F    = *(std::next(M->begin(), address.getFnIndex()));
  BasicBlock &B  = *(std::next(F.begin(), address.getBBIndex()));
  Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  CmpInst *cmp = cast<CmpInst>(&I);

  assert(isa<CmpInst>(cmp) &&
         "Expected instruction to be cmp instruction: Instruction::ICmp or Instruction::FCmp");

  /// NOTE: Create a new CmpInst with the same operands as original instruction but with
  /// mutated comparison predicate.
  Optional<CmpInst::Predicate> mutatedPredicate = getMutatedPredicate(cmp->getPredicate());
  assert(mutatedPredicate.hasValue());

  CmpInst *replacement = CmpInst::Create(cmp->getOpcode(),
                                         mutatedPredicate.getValue(),
                                         cmp->getOperand(0),
                                         cmp->getOperand(1),
                                         cmp->getName());

  replacement->insertAfter(cmp);
  cmp->replaceAllUsesWith(replacement);
  cmp->eraseFromParent();

  return replacement;
}

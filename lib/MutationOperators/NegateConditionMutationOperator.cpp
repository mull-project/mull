#include "MutationOperators/NegateConditionMutationOperator.h"

#include "MutationPoint.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace Mutang;

static int GetFunctionIndex(llvm::Function *function) {
  auto PM = function->getParent();

  auto FII = std::find_if(PM->begin(), PM->end(),
                          [function] (llvm::Function &f) {
                            return &f == function;
                          });

  assert(FII != PM->end() && "Expected function to be found in module");
  int FIndex = std::distance(PM->begin(), FII);

  return FIndex;
}

llvm::CmpInst::Predicate
NegateConditionMutationOperator::negatedCmpInstPredicate(llvm::CmpInst::Predicate predicate) {

  switch (predicate) {
    /// FCMP_FALSE =  0,  ///< 0 0 0 0    Always false (always folded)
    case CmpInst::FCMP_FALSE: {
      return CmpInst::FCMP_TRUE;
    }

      /* TO IMPLEMENT:
      FCMP_OEQ =  1,  ///< 0 0 0 1    True if ordered and equal
      FCMP_OGT   =  2,  ///< 0 0 1 0    True if ordered and greater than
      FCMP_OGE   =  3,  ///< 0 0 1 1    True if ordered and greater than or equal
      FCMP_OLT   =  4,  ///< 0 1 0 0    True if ordered and less than
      FCMP_OLE   =  5,  ///< 0 1 0 1    True if ordered and less than or equal
      FCMP_ONE   =  6,  ///< 0 1 1 0    True if ordered and operands are unequal
      FCMP_ORD   =  7,  ///< 0 1 1 1    True if ordered (no nans)
      FCMP_UNO   =  8,  ///< 1 0 0 0    True if unordered: isnan(X) | isnan(Y)
      FCMP_UEQ   =  9,  ///< 1 0 0 1    True if unordered or equal
      FCMP_UGT   = 10,  ///< 1 0 1 0    True if unordered or greater than
      FCMP_UGE   = 11,  ///< 1 0 1 1    True if unordered, greater than, or equal
      FCMP_ULT   = 12,  ///< 1 1 0 0    True if unordered or less than
      FCMP_ULE   = 13,  ///< 1 1 0 1    True if unordered, less than, or equal
      FCMP_UNE   = 14,  ///< 1 1 1 0    True if unordered or not equal
      FCMP_TRUE  = 15,  ///< 1 1 1 1    Always true (always folded)
      FIRST_FCMP_PREDICATE = FCMP_FALSE,
      LAST_FCMP_PREDICATE = FCMP_TRUE,
      BAD_FCMP_PREDICATE = FCMP_TRUE + 1,
       */

    case CmpInst::ICMP_EQ: {
      return CmpInst::ICMP_NE;
    }

    case CmpInst::ICMP_NE: {
      return CmpInst::ICMP_EQ;
    }

    case CmpInst::ICMP_UGT: {
      return CmpInst::ICMP_ULE;
    }

    case CmpInst::ICMP_UGE: {
      return CmpInst::ICMP_ULT;
    }

    case CmpInst::ICMP_ULT: {
      return CmpInst::ICMP_UGE;
    }

    case CmpInst::ICMP_ULE: {
      return CmpInst::ICMP_UGT;
    }

    case CmpInst::ICMP_SGT: {
      return CmpInst::ICMP_SLE;
    }

    case CmpInst::ICMP_SGE: {
      return CmpInst::ICMP_SLT;
    }

    case CmpInst::ICMP_SLT: {
      return CmpInst::ICMP_SGE;
    }

    case CmpInst::ICMP_SLE: {
      return CmpInst::ICMP_SGT;
    }

    /* TO IMPLEMENT:
     FIRST_ICMP_PREDICATE = ICMP_EQ,
     LAST_ICMP_PREDICATE = ICMP_SLE,
     BAD_ICMP_PREDICATE = ICMP_SLE + 1
     */

    default: {
      printf("Unsupported predicate: %d\n", predicate);
      llvm_unreachable("Unsupported predicate!");
    }
  }
}

std::vector<MutationPoint *> NegateConditionMutationOperator::getMutationPoints(
                                                                    llvm::Function *function) {
  int functionIndex = GetFunctionIndex(function);
  int basicBlockIndex = 0;

  std::vector<MutationPoint *> mutationPoints;

  for (auto &basicBlock : function->getBasicBlockList()) {

    int instructionIndex = 0;

    for (auto &instruction : basicBlock.getInstList()) {
      if (canBeApplied(instruction)) {
        MutationPointAddress address(functionIndex, basicBlockIndex, instructionIndex);
        auto mutationPoint = new MutationPoint(this, address, &instruction);
        mutationPoints.push_back(mutationPoint);
      }
      instructionIndex++;
    }
    basicBlockIndex++;
  }

  return mutationPoints;
}

bool NegateConditionMutationOperator::canBeApplied(Value &V) {

  if (__unused CmpInst *cmpOp = dyn_cast<CmpInst>(&V)) {
    return true;
  }

  return false;
}

llvm::Value *NegateConditionMutationOperator::applyMutation(Module *M, MutationPointAddress address, Value &_V) {
  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module

  llvm::Function &F    = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B  = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  CmpInst *cmpInstruction = cast<CmpInst>(&I);

  assert(CmpInst::classof(cmpInstruction) &&
         "Expected instruction to be cmp instruction: Instruction::ICmp or Instruction::FCmp");

  /// NOTE: Create a new CmpInst with the same operands as original instruction but with
  /// negated comparison predicate.
  CmpInst::Predicate negatedPredicate =
    NegateConditionMutationOperator::negatedCmpInstPredicate(cmpInstruction->getPredicate());

  ICmpInst *replacement = new ICmpInst(negatedPredicate,
                                       cmpInstruction->getOperand(0),
                                       cmpInstruction->getOperand(1),
                                       cmpInstruction->getName());

  replacement->insertAfter(cmpInstruction);
  cmpInstruction->replaceAllUsesWith(replacement);
  cmpInstruction->eraseFromParent();

  return replacement;
}

Value *NegateConditionMutationOperator::revertMutation(Value &V)  {
  LLVM_BUILTIN_UNREACHABLE;
  return nullptr;
}

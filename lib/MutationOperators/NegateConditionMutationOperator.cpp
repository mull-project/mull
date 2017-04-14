#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/MutationOperatorFilter.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

const std::string NegateConditionMutationOperator::ID =
  "negate_mutation_operator";

///
/// Comparison instructions emitted for explicit and implicit comparisons
/// We should only apply mutations on explicit comparisons.
/// e.g.:
///
///   int a, b;
///   if (a < b)   // Explicit comparison. Applying mutation
///
///   void *ptr = nullptr;
///   if (ptr)     // Implicit comparison. Not applying mutation
///
///   void *ptr = nullptr;
///   delete ptr;  // Emits comparison instruction. Also not applying.
///
/// Based on observation we know that all implicit comparison instructions
/// are named `toboolX`, where `X` is an empty string or a number.
/// Number used when there is more than one instruction pre basic block.
/// `delete` instructions named `isnullX`, where `X` follows the same
/// pattern as `tobool`'s `X`.
///

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

    /// Ordered comparisons

    /// == -> !=
    case CmpInst::FCMP_OEQ: {
      printf(" FCMP_OEQ => FCMP_ONE \n");
      return CmpInst::FCMP_ONE;
    }

    /// != -> ==
    case CmpInst::FCMP_ONE: {
      printf(" FCMP_ONE => FCMP_OEQ \n");
      return CmpInst::FCMP_OEQ;
    }

    /// > -> <=
    case CmpInst::FCMP_OGT: {
      printf(" FCMP_OGT => FCMP_OLE \n");
      return CmpInst::FCMP_OLE;
    }

    /// <= -> >
    case CmpInst::FCMP_OLE: {
      printf(" FCMP_OLE => FCMP_OGT \n");
      return CmpInst::FCMP_OGT;
    }

    /// >= -> <
    case CmpInst::FCMP_OGE: {
      printf(" FCMP_OGE => FCMP_OLT \n");
      return CmpInst::FCMP_OLT;
    }

    /// < -> >=
    case CmpInst::FCMP_OLT: {
      printf(" FCMP_OLT => FCMP_OGE \n");
      return CmpInst::FCMP_OGE;
    }

    /// Unordered comparisons

    /// == -> !=
    case CmpInst::FCMP_UEQ: {
      printf(" FCMP_UEQ => FCMP_UNE \n");
      return CmpInst::FCMP_UNE;
    }

    /// != -> ==
    case CmpInst::FCMP_UNE: {
      printf(" FCMP_UNE => FCMP_UEQ \n");
      return CmpInst::FCMP_UEQ;
    }

    /// >= -> <
    case CmpInst::FCMP_UGE: {
      printf(" FCMP_UGE => FCMP_ULT \n");
      return CmpInst::FCMP_ULT;
    }

    /// < -> >=
    case CmpInst::FCMP_ULT: {
      printf(" FCMP_ULT => FCMP_UGE \n");
      return CmpInst::FCMP_UGE;
    }

    /// > -> <=
    case CmpInst::FCMP_UGT: {
      printf(" FCMP_UGT => FCMP_ULE \n");
      return CmpInst::FCMP_ULE;
    }

    /// < -> >=
    case CmpInst::FCMP_ULE: {
      printf(" FCMP_ULE => FCMP_UGT \n");
      return CmpInst::FCMP_UGT;
    }

    /// Normal comparisons

    /// == -> !=
    case CmpInst::ICMP_EQ: {
      printf(" ICMP_EQ => ICMP_NE \n");
      return CmpInst::ICMP_NE;
    }

    /// != -> ==
    case CmpInst::ICMP_NE: {
      printf(" ICMP_NE => ICMP_EQ \n");
      return CmpInst::ICMP_EQ;
    }

    /// Unsigned: > -> <=
    case CmpInst::ICMP_UGT: {
      printf(" ICMP_UGT => ICMP_ULE \n");
      return CmpInst::ICMP_ULE;
    }

    /// Unsigned: <= -> >
    case CmpInst::ICMP_ULE: {
      printf(" ICMP_ULE => ICMP_UGT \n");
      return CmpInst::ICMP_UGT;
    }

    /// Unsigned: >= -> <
    case CmpInst::ICMP_UGE: {
      printf(" ICMP_UGE => ICMP_ULT \n");
      return CmpInst::ICMP_ULT;
    }

    /// Unsigned: < -> >=
    case CmpInst::ICMP_ULT: {
      printf(" ICMP_ULT => ICMP_UGE \n");
      return CmpInst::ICMP_UGE;
    }

    /// Signed: > -> <=
    case CmpInst::ICMP_SGT: {
      printf(" ICMP_SGT => ICMP_SLE \n");
      return CmpInst::ICMP_SLE;
    }

    /// Signed: <= -> >
    case CmpInst::ICMP_SLE: {
      printf(" ICMP_SLE => ICMP_SGT \n");
      return CmpInst::ICMP_SGT;
    }

    /// Signed: >= -> <
    case CmpInst::ICMP_SGE: {
      printf(" ICMP_SGE => ICMP_SLT \n");
      return CmpInst::ICMP_SLT;
    }

    /// Signed: < -> >=
    case CmpInst::ICMP_SLT: {
      printf(" ICMP_SLT => ICMP_SGE \n");
      return CmpInst::ICMP_SGE;
    }

    // Etc
    case CmpInst::FCMP_FALSE: {
      printf(" FCMP_FALSE => FCMP_TRUE \n");
      return CmpInst::FCMP_TRUE;
    }

    case CmpInst::FCMP_TRUE: {
      printf(" FCMP_TRUE => FCMP_FALSE \n");
      return CmpInst::FCMP_FALSE;
    }

    default: {
      Logger::error() << "Unsupported predicate: " << predicate << '\n';
      llvm_unreachable("Unsupported predicate!");
    }
  }
}

std::vector<MutationPoint *>
NegateConditionMutationOperator::getMutationPoints(const Context &context,
                                                   llvm::Function *function,
                                                   MutationOperatorFilter &filter) {
  int functionIndex = GetFunctionIndex(function);
  int basicBlockIndex = 0;

  std::vector<MutationPoint *> mutationPoints;

  for (auto &basicBlock : function->getBasicBlockList()) {

    int instructionIndex = 0;

    for (auto &instruction : basicBlock.getInstList()) {
      if (canBeApplied(instruction) && !filter.shouldSkipInstruction(&instruction)) {
        auto moduleID = instruction.getModule()->getModuleIdentifier();
        MullModule *module = context.moduleWithIdentifier(moduleID);

        MutationPointAddress address(functionIndex, basicBlockIndex, instructionIndex);
        auto mutationPoint = new MutationPoint(this, address, &instruction, module);
        mutationPoints.push_back(mutationPoint);
      }
      instructionIndex++;
    }
    basicBlockIndex++;
  }

  return mutationPoints;
}

bool NegateConditionMutationOperator::canBeApplied(Value &V) {

  if (CmpInst *cmpOp = dyn_cast<CmpInst>(&V)) {
    if (cmpOp->getName().startswith("tobool")) {
      return false;
    }

    if (cmpOp->getName().startswith("isnull")) {
      return false;
    }

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

  CmpInst *replacement = CmpInst::Create(cmpInstruction->getOpcode(),
                                         negatedPredicate,
                                         cmpInstruction->getOperand(0),
                                         cmpInstruction->getOperand(1),
                                         cmpInstruction->getName());

  replacement->insertAfter(cmpInstruction);
  cmpInstruction->replaceAllUsesWith(replacement);
  cmpInstruction->eraseFromParent();

  return replacement;
}

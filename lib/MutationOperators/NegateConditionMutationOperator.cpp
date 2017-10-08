#include "MutationOperators/NegateConditionMutationOperator.h"
#include "MutationOperators/MutationOperatorFilter.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

class mull::NegateConditionMutationPoint: public IMutationPoint {
  NegateConditionMutationOperator *mutationOperator;
  MutationPointAddress address;
  llvm::Value *OriginalValue;
  MullModule *module;
  std::string uniqueIdentifier;
  CmpInst::Predicate originalPredicate;
  CmpInst::Predicate negatedPredicate;
public:
  NegateConditionMutationPoint(NegateConditionMutationOperator *op,
                               MutationPointAddress address,
                               Value *Val,
                               MullModule *m,
                               CmpInst::Predicate originalPredicate,
                               CmpInst::Predicate negatedPredicate) :
  mutationOperator(op),
  address(address),
  OriginalValue(Val),
  module(m),
  originalPredicate(originalPredicate),
  negatedPredicate(negatedPredicate)
  {
    std::string moduleID = module->getUniqueIdentifier();
    std::string addressID = address.getIdentifier();
    std::string operatorID = mutationOperator->uniqueID();

    uniqueIdentifier = moduleID + "_" + addressID + "_" + operatorID;
  }

  NegateConditionMutationOperator *getOperator() {
    return mutationOperator;
  }

  MutationPointAddress getAddress() {
    return address;
  }

  MutationPointAddress getAddress() const {
    return address;
  }

  Value *getOriginalValue() {
    return OriginalValue;
  }

  Value *getOriginalValue() const {
    return OriginalValue;
  }

  MutationOperator *getOperator() const {
    return mutationOperator;
  }

  std::unique_ptr<llvm::Module> cloneModuleAndApplyMutation() {
    auto copyForMutation = CloneModule(module->getModule());
    assert(copyForMutation);

    mutationOperator->applyMutation2(copyForMutation.get(),
                                     this);

    return copyForMutation;
  }

  std::string getUniqueIdentifier() {
    return uniqueIdentifier;
  }

  std::string getUniqueIdentifier() const {
    return uniqueIdentifier;
  }

  CmpInst::Predicate getOriginalPredicate() {
    return originalPredicate;
  }

  CmpInst::Predicate getNegatedPredicate() {
    return negatedPredicate;
  }

  std::string getDiagnostics() {
    std::stringstream diagnostics;
    diagnostics << "Negate Condition: Replaced ";
    diagnostics << describePredicate(originalPredicate);
    diagnostics << " with ";
    diagnostics << describePredicate(negatedPredicate);
    diagnostics << " (" << originalPredicate << "->" << negatedPredicate << ")";
    diagnostics << "\n";

    return diagnostics.str();
  }

  std::string describePredicate(CmpInst::Predicate predicate) {
    switch (predicate) {

      case CmpInst::FCMP_FALSE:
        return "Always false";

      case CmpInst::FCMP_OEQ:
        return "Ordered equal";

//        FCMP_OGT   =  2,  ///< 0 0 1 0    True if ordered and greater than
//        FCMP_OGE   =  3,  ///< 0 0 1 1    True if ordered and greater than or equal
//        FCMP_OLT   =  4,  ///< 0 1 0 0    True if ordered and less than
//        FCMP_OLE   =  5,  ///< 0 1 0 1    True if ordered and less than or equal
//        FCMP_ONE   =  6,  ///< 0 1 1 0    True if ordered and operands are unequal
//        FCMP_ORD   =  7,  ///< 0 1 1 1    True if ordered (no nans)
//        FCMP_UNO   =  8,  ///< 1 0 0 0    True if unordered: isnan(X) | isnan(Y)
//        FCMP_UEQ   =  9,  ///< 1 0 0 1    True if unordered or equal
//        FCMP_UGT   = 10,  ///< 1 0 1 0    True if unordered or greater than
//        FCMP_UGE   = 11,  ///< 1 0 1 1    True if unordered, greater than, or equal
//        FCMP_ULT   = 12,  ///< 1 1 0 0    True if unordered or less than
//        FCMP_ULE   = 13,  ///< 1 1 0 1    True if unordered, less than, or equal
//        FCMP_UNE   = 14,  ///< 1 1 1 0    True if unordered or not equal
//        FCMP_TRUE  = 15,  ///< 1 1 1 1    Always true (always folded)
//        FIRST_FCMP_PREDICATE = FCMP_FALSE,
//        LAST_FCMP_PREDICATE = FCMP_TRUE,
//        BAD_FCMP_PREDICATE = FCMP_TRUE + 1,

      case CmpInst::ICMP_EQ:
        return "==";

      case CmpInst::ICMP_NE:
        return "!=";

//        ICMP_UGT   = 34,  ///< unsigned greater than
//        ICMP_UGE   = 35,  ///< unsigned greater or equal
//        ICMP_ULT   = 36,  ///< unsigned less than
//        ICMP_ULE   = 37,  ///< unsigned less or equal
//        ICMP_SGT   = 38,  ///< signed greater than
//        ICMP_SGE   = 39,  ///< signed greater or equal
//        ICMP_SLT   = 40,  ///< signed less than
//        ICMP_SLE   = 41,  ///< signed less or equal
//        FIRST_ICMP_PREDICATE = ICMP_EQ,
//        LAST_ICMP_PREDICATE = ICMP_SLE,
//        BAD_ICMP_PREDICATE = ICMP_SLE + 1

      default:
        return "TODO";
    }
  }
};

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
      return CmpInst::FCMP_ONE;
    }

    /// != -> ==
    case CmpInst::FCMP_ONE: {
      return CmpInst::FCMP_OEQ;
    }

    /// > -> <=
    case CmpInst::FCMP_OGT: {
      return CmpInst::FCMP_OLE;
    }

    /// <= -> >
    case CmpInst::FCMP_OLE: {
      return CmpInst::FCMP_OGT;
    }

    /// >= -> <
    case CmpInst::FCMP_OGE: {
      return CmpInst::FCMP_OLT;
    }

    /// < -> >=
    case CmpInst::FCMP_OLT: {
      return CmpInst::FCMP_OGE;
    }

    /// Unordered comparisons

    /// == -> !=
    case CmpInst::FCMP_UEQ: {
      return CmpInst::FCMP_UNE;
    }

    /// != -> ==
    case CmpInst::FCMP_UNE: {
      return CmpInst::FCMP_UEQ;
    }

    /// >= -> <
    case CmpInst::FCMP_UGE: {
      return CmpInst::FCMP_ULT;
    }

    /// < -> >=
    case CmpInst::FCMP_ULT: {
      return CmpInst::FCMP_UGE;
    }

    /// > -> <=
    case CmpInst::FCMP_UGT: {
      return CmpInst::FCMP_ULE;
    }

    /// < -> >=
    case CmpInst::FCMP_ULE: {
      return CmpInst::FCMP_UGT;
    }

    /// Normal comparisons

    /// == -> !=
    case CmpInst::ICMP_EQ: {
      return CmpInst::ICMP_NE;
    }

    /// != -> ==
    case CmpInst::ICMP_NE: {
      return CmpInst::ICMP_EQ;
    }

    /// Unsigned: > -> <=
    case CmpInst::ICMP_UGT: {
      return CmpInst::ICMP_ULE;
    }

    /// Unsigned: <= -> >
    case CmpInst::ICMP_ULE: {
      return CmpInst::ICMP_UGT;
    }

    /// Unsigned: >= -> <
    case CmpInst::ICMP_UGE: {
      return CmpInst::ICMP_ULT;
    }

    /// Unsigned: < -> >=
    case CmpInst::ICMP_ULT: {
      return CmpInst::ICMP_UGE;
    }

    /// Signed: > -> <=
    case CmpInst::ICMP_SGT: {
      return CmpInst::ICMP_SLE;
    }

    /// Signed: <= -> >
    case CmpInst::ICMP_SLE: {
      return CmpInst::ICMP_SGT;
    }

    /// Signed: >= -> <
    case CmpInst::ICMP_SGE: {
      return CmpInst::ICMP_SLT;
    }

    /// Signed: < -> >=
    case CmpInst::ICMP_SLT: {
      return CmpInst::ICMP_SGE;
    }

    // Etc
    case CmpInst::FCMP_FALSE: {
      return CmpInst::FCMP_TRUE;
    }

    case CmpInst::FCMP_TRUE: {
      return CmpInst::FCMP_FALSE;
    }

    default: {
      Logger::error() << "Unsupported predicate: " << predicate << '\n';
      llvm_unreachable("Unsupported predicate!");
    }
  }
}

std::vector<IMutationPoint *>
NegateConditionMutationOperator::getMutationPoints(const Context &context,
                                                   llvm::Function *function,
                                                   MutationOperatorFilter &filter) {
  int functionIndex = GetFunctionIndex(function);
  int basicBlockIndex = 0;

  std::vector<IMutationPoint *> mutationPoints;

  for (auto &basicBlock : function->getBasicBlockList()) {

    int instructionIndex = 0;

    for (auto &instruction : basicBlock.getInstList()) {
      if (filter.shouldSkipInstruction(&instruction)) {
        instructionIndex++;

        continue;
      }

      CmpInst::Predicate originalPredicate, negatedPredicate;
      if (canBeApplied(instruction, &originalPredicate, &negatedPredicate)) {
        auto moduleID = instruction.getModule()->getModuleIdentifier();
        MullModule *module = context.moduleWithIdentifier(moduleID);

        MutationPointAddress address(functionIndex, basicBlockIndex, instructionIndex);

        auto mutationPoint =
          new NegateConditionMutationPoint(this,
                                           address,
                                           &instruction,
                                           module,
                                           originalPredicate,
                                           negatedPredicate);

        mutationPoints.push_back(mutationPoint);
      }

      instructionIndex++;
    }
    basicBlockIndex++;
  }

  return mutationPoints;
}

bool NegateConditionMutationOperator::canBeApplied(llvm::Value &V) {
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

bool
NegateConditionMutationOperator::canBeApplied(Value &V,
                                              CmpInst::Predicate *outPredicate,
                                              CmpInst::Predicate *outNegatedPredicate) {

  if (CmpInst *cmpOp = dyn_cast<CmpInst>(&V)) {
    if (cmpOp->getName().startswith("tobool")) {
      return false;
    }

    if (cmpOp->getName().startswith("isnull")) {
      return false;
    }

    assert(outPredicate && outNegatedPredicate);
    CmpInst::Predicate negatedPredicate =
      NegateConditionMutationOperator::negatedCmpInstPredicate(cmpOp->getPredicate());

    *outPredicate = cmpOp->getPredicate();
    *outNegatedPredicate = negatedPredicate;

    return true;
  }

  return false;
}

llvm::Value *
NegateConditionMutationOperator::applyMutation(Module *M,
                                               MutationPointAddress address,
                                               Value &_V) {

  llvm::Function &F    = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B  = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  CmpInst *cmpInstruction = cast<CmpInst>(&I);

  assert(CmpInst::classof(cmpInstruction) &&
         "Expected instruction to be cmp instruction: Instruction::ICmp or Instruction::FCmp");

  CmpInst::Predicate negatedPredicate =
    negatedCmpInstPredicate(cmpInstruction->getPredicate());

    /// NOTE: Create a new CmpInst with the same operands as original instruction but with
    /// negated comparison predicate.
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

llvm::Value *
NegateConditionMutationOperator::applyMutation2(Module *M,
                                                NegateConditionMutationPoint *mp) {

  MutationPointAddress address = mp->getAddress();
  llvm::Function &F    = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B  = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  CmpInst *cmpInstruction = cast<CmpInst>(&I);

  assert(CmpInst::classof(cmpInstruction) &&
         "Expected instruction to be cmp instruction: Instruction::ICmp or Instruction::FCmp");

  /// NOTE: Create a new CmpInst with the same operands as original instruction but with
  /// negated comparison predicate.
  CmpInst *replacement = CmpInst::Create(cmpInstruction->getOpcode(),
                                         mp->getNegatedPredicate(),
                                         cmpInstruction->getOperand(0),
                                         cmpInstruction->getOperand(1),
                                         cmpInstruction->getName());

  replacement->insertAfter(cmpInstruction);
  cmpInstruction->replaceAllUsesWith(replacement);
  cmpInstruction->eraseFromParent();

  return replacement;
}

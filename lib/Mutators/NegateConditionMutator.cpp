#include "Mutators/NegateConditionMutator.h"

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

const std::string NegateConditionMutator::ID = "negate_mutator";
const std::string NegateConditionMutator::description = "Negates conditionals, e.g.: != to ==, > to <=";

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

llvm::CmpInst::Predicate
NegateConditionMutator::negatedCmpInstPredicate(llvm::CmpInst::Predicate predicate) {

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
  diagnostics << " (" << originalPredicate << "->" << negatedPredicate << ")";
  diagnostics << "\n";

  return diagnostics.str();
}


MutationPoint *
NegateConditionMutator::getMutationPoint(MullModule *module,
                                         llvm::Function *function,
                                         llvm::Instruction *instruction,
                                         SourceLocation &sourceLocation,
                                         MutationPointAddress &address) {
  if (canBeApplied(*instruction)) {
    CmpInst *cmpOp = dyn_cast<CmpInst>(instruction);
    assert(cmpOp);

    if (cmpOp->getPredicate() == CmpInst::FCMP_UNO) {
      return nullptr;
    }

    std::string diagnostics =
      getDiagnostics(cmpOp->getPredicate(),
                     negatedCmpInstPredicate(cmpOp->getPredicate()));

    return new MutationPoint(this, address, instruction, function, diagnostics, sourceLocation, module);
  }
  return nullptr;
}

bool NegateConditionMutator::canBeApplied(Value &V) {

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

llvm::Value *NegateConditionMutator::applyMutation(Function *function,
                                                   MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  CmpInst *cmpInstruction = cast<CmpInst>(&I);

  assert(CmpInst::classof(cmpInstruction) &&
         "Expected instruction to be cmp instruction: Instruction::ICmp or Instruction::FCmp");

  /// NOTE: Create a new CmpInst with the same operands as original instruction but with
  /// negated comparison predicate.
  CmpInst::Predicate negatedPredicate =
    NegateConditionMutator::negatedCmpInstPredicate(cmpInstruction->getPredicate());

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

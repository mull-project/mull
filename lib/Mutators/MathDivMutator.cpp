#include "Mutators/MathDivMutator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationPoint.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfoMetadata.h>

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

const std::string MathDivMutator::ID = "math_div_mutator";

MutationPoint *
MathDivMutator::getMutationPoint(MullModule *module,
                                 llvm::Function *function,
                                 llvm::Instruction *instruction,
                                 SourceLocation &sourceLocation,
                                 MutationPointAddress &address) {
  if (canBeApplied(*instruction)) {
    std::string diagnostics = "Math Div: replaced / with *";
    return new MutationPoint(this, address, instruction, function, diagnostics, sourceLocation, module);
  }

  return nullptr;
}

bool MathDivMutator::canBeApplied(Value &V) {
  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();

    if (Opcode == Instruction::UDiv ||
        Opcode == Instruction::SDiv ||
        Opcode == Instruction::FDiv) {
      return true;
    }
  }

  return false;
}

llvm::Value *MathDivMutator::applyMutation(Function *function,
                                           MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  /// TODO: Take care of NUW/NSW
  BinaryOperator *binaryOperator = cast<BinaryOperator>(&I);
  assert(binaryOperator->getOpcode() == Instruction::SDiv ||
         binaryOperator->getOpcode() == Instruction::UDiv ||
         binaryOperator->getOpcode() == Instruction::FDiv);

  auto type = Instruction::Mul;
  if (binaryOperator->getOpcode() == Instruction::FDiv) {
    type = Instruction::FMul;
  }

  /// NOTE: Create a new BinaryOperator with the same name as existing one
  BinaryOperator *replacement = BinaryOperator::Create(type,
                                                       binaryOperator->getOperand(0),
                                                       binaryOperator->getOperand(1),
                                                       binaryOperator->getName());
  assert(replacement);
  if (binaryOperator->hasNoUnsignedWrap()) {
    replacement->setHasNoUnsignedWrap();
  }

  if (binaryOperator->hasNoSignedWrap()) {
    replacement->setHasNoSignedWrap();
  }

  /// NOTE: If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  /// TODO: Check behaviour of 'unnamed' instructions (e.g.: %0, %2, etc.)
  binaryOperator->setName("");

  replacement->insertAfter(binaryOperator);
  binaryOperator->replaceAllUsesWith(replacement);
  binaryOperator->eraseFromParent();

  return replacement;
}

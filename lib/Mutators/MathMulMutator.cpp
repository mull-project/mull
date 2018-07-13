#include "Mutators/MathMulMutator.h"

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

const std::string MathMulMutator::ID = "math_mul_mutator";

MutationPoint *
MathMulMutator::getMutationPoint(MullModule *module,
                                 MutationPointAddress &address,
                                 llvm::Instruction *instruction,
                                 SourceLocation &sourceLocation) {
  if (canBeApplied(*instruction)) {
    std::string diagnostics = "Math Mul: replaced * with /";
    return new MutationPoint(this, address, instruction, module, diagnostics, sourceLocation);
  }
  return nullptr;
}

bool MathMulMutator::canBeApplied(Value &V) {
  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();

    if (Opcode == Instruction::Mul || Opcode == Instruction::FMul) {
      return true;
    }
  }

  return false;
}

llvm::Value *MathMulMutator::applyMutation(llvm::Module *module,
                                           MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(module);

  /// TODO: Take care of NUW/NSW
  BinaryOperator *binaryOperator = cast<BinaryOperator>(&I);
  assert(binaryOperator->getOpcode() == Instruction::Mul ||
         binaryOperator->getOpcode() == Instruction::FMul);

  /// Seems fine to always mutate Mul to SDiv.
  auto type = Instruction::SDiv;
  if (binaryOperator->getOpcode() == Instruction::FMul) {
    type = Instruction::FDiv;
  }

  /// NOTE: Create a new BinaryOperator with the same name as existing one

  Instruction *replacement = BinaryOperator::Create(type,
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

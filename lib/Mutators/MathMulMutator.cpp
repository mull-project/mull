#include "mull/Mutators/MathMulMutator.h"

#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Operator.h>

#include <fstream>
#include <iterator>

using namespace llvm;
using namespace mull;

const std::string MathMulMutator::ID = "math_mul_mutator";
const std::string MathMulMutator::description = "Replaces * with /";

bool MathMulMutator::canBeApplied(Value &V) {
  if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(&V)) {
    BinaryOperator::BinaryOps Opcode = BinOp->getOpcode();

    if (Opcode == Instruction::Mul || Opcode == Instruction::FMul) {
      return true;
    }
  }

  return false;
}

void MathMulMutator::applyMutation(Function *function,
                                   const MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  auto *binaryOperator = cast<BinaryOperator>(&I);
  assert(binaryOperator->getOpcode() == Instruction::Mul ||
         binaryOperator->getOpcode() == Instruction::FMul);

  /// Seems fine to always mutate Mul to SDiv.
  auto type = Instruction::SDiv;
  if (binaryOperator->getOpcode() == Instruction::FMul) {
    type = Instruction::FDiv;
  }

  /// NOTE: Create a new BinaryOperator with the same name as existing one

  Instruction *replacement = BinaryOperator::Create(
      type, binaryOperator->getOperand(0), binaryOperator->getOperand(1),
      binaryOperator->getName());
  assert(replacement);
  /// TODO: Take care of NUW/NSW
  /// SDiv and FDiv do not support NUW/NSW for some reason
  /// Need to double check the behavior across different versions of LLVM

  /// NOTE: If I add a named instruction, and the name already exist
  /// in a basic block, then LLVM will make another unique name of it
  /// To prevent this name change we need to 'drop' the existing old name
  /// TODO: Check behaviour of 'unnamed' instructions (e.g.: %0, %2, etc.)
  binaryOperator->setName("");

  replacement->insertAfter(binaryOperator);
  binaryOperator->replaceAllUsesWith(replacement);
  binaryOperator->eraseFromParent();
}

std::vector<MutationPoint *>
MathMulMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;
  for (auto &instruction : instructions(function)) {
    if (!canBeApplied(instruction)) {
      continue;
    }

    std::string diagnostics = "Math Mul: replaced * with /";
    auto point =
        new MutationPoint(this, &instruction, diagnostics, "/", bitcode);
    mutations.push_back(point);
  }
  return mutations;
}

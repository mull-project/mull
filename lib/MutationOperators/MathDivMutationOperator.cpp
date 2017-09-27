#include "MutationOperators/MathDivMutationOperator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationOperators/MutationOperatorFilter.h"
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

const std::string MathDivMutationOperator::ID = "math_div_mutation_operator";

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

std::vector<MutationPoint *>
MathDivMutationOperator::getMutationPoints(const Context &context,
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

bool MathDivMutationOperator::canBeApplied(Value &V) {
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

llvm::Value *MathDivMutationOperator::applyMutation(Module *M,
                                                    MutationPointAddress address,
                                                    Value &_V) {

  /// In the following V argument is not used. Eventually it will be removed from
  /// this method's signature because it will be not relevant
  /// when mutations will be applied on copies of original module
  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

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

#include "MutationOperators/ScalarValueMutationOperator.h"

#include "Context.h"
#include "Logger.h"
#include "MutationOperators/MutationOperatorFilter.h"
#include "MutationPoint.h"

#include <llvm/IR/Constants.h>
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

enum ScalarValueMutationType {
  None = 1,
  Int,
  Float
};

static
ScalarValueMutationType findPossibleApplication(Value &V,
                                                int64_t *outIntValue,
                                                double *outDoubleValue);

const std::string ScalarValueMutationOperator::ID = "scalar_value_mutation_operator";

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

static
void EnumerateInstructions(Function &function,
                           const std::function <void (Instruction &, int, int)>& block) {
  int basicBlockIndex = 0;
  for (auto &basicBlock : function.getBasicBlockList()) {
    int instructionIndex = 0;

    for (auto &instruction : basicBlock.getInstList()) {
      block(instruction, basicBlockIndex, instructionIndex);

      instructionIndex++;
    }
    basicBlockIndex++;
  }
}

std::vector<MutationPoint *>
ScalarValueMutationOperator::getMutationPoints(const Context &context,
                                           llvm::Function *function,
                                           MutationOperatorFilter &filter) {
  int functionIndex = GetFunctionIndex(function);

  std::vector<MutationPoint *> mutationPoints;

  EnumerateInstructions(*function, [&](Instruction &instr, int bbIndex, int iIndex) {
    if (filter.shouldSkipInstruction(&instr)) {
      return;
    }

    int64_t intValue = 0;
    double doubleValue = 0.0;

    ScalarValueMutationType mutationType =
      findPossibleApplication(instr, &intValue, &doubleValue);
    if (mutationType == ScalarValueMutationType::None) {
      return;
    }

    auto moduleID = instr.getModule()->getModuleIdentifier();
    MullModule *module = context.moduleWithIdentifier(moduleID);

    std::stringstream diagstream;
    diagstream << "Scalar Value Replacement: ";

    if (mutationType == ScalarValueMutationType::Int) {
      diagstream << intValue;
    } else {
      diagstream << doubleValue;
    }

    std::string diagnostics = diagstream.str();

    MutationPointAddress address(functionIndex, bbIndex, iIndex);
    auto mutationPoint =
      new MutationPoint(this, address, &instr, module, diagnostics);

    mutationPoints.push_back(mutationPoint);
  });

  return mutationPoints;
}

/// Currently only used by SimpleTestFinder.
bool ScalarValueMutationOperator::canBeApplied(Value &V) {
  return findPossibleApplication(V, nullptr, nullptr) != ScalarValueMutationType::None;
}

static
ScalarValueMutationType findPossibleApplication(Value &V,
                                                int64_t *outIntValue,
                                                double *outDoubleValue) {
  Instruction *instruction = dyn_cast<Instruction>(&V);
  assert(instruction);

  unsigned opcode = instruction->getOpcode();

  if ((opcode >= Instruction::BinaryOpsBegin &&
       opcode < Instruction::BinaryOpsEnd) ||
      opcode == Instruction::Store ||
      opcode == Instruction::FCmp ||
      opcode == Instruction::ICmp ||
      opcode == Instruction::Ret ||
      opcode == Instruction::Call ||
      opcode == Instruction::Invoke) {

    for (Value *operand: instruction->operands()) {
      if (ConstantInt *constantInt = dyn_cast<ConstantInt>(operand)) {
        auto intValue = constantInt->getValue();

        /// Skip big number because getSExtValue throws otherwise.
        /// TODO: consider these edge cases in unit tests.
        if (intValue.getNumWords() > 1) {
          continue;
        }

        if (outIntValue) {
          *outIntValue = intValue.getSExtValue();
        }

        return ScalarValueMutationType::Int;
      }

      if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(operand)) {
        auto floatValue = constantFloat->getValueAPF();

        if (outDoubleValue) {
          *outDoubleValue = floatValue.convertToDouble();
        }

        return ScalarValueMutationType::Float;
      }
    }
  }

  return ScalarValueMutationType::None;
}

static ConstantInt *getReplacementInt(ConstantInt *constantInt) {
  auto type = constantInt->getType();

  auto intValue = constantInt->getValue();

  // TODO: Refactor to avoid unnecessary construction.
  APInt replacementIntValue;

  // TODO: Review the rules for mutation.
  // TODO: Didn't find a better way of testing APInt for zero-ness.
  if (intValue.isNegative() == false && intValue.isStrictlyPositive() == false) {
    replacementIntValue = APInt(intValue.getBitWidth(), 1);
  } else {
    replacementIntValue = APInt(intValue.getBitWidth(), 0);
  }

  ConstantInt *replacement = dyn_cast<ConstantInt>(ConstantInt::get(type, replacementIntValue));

  return replacement;
}

static ConstantFP *getReplacementFloat(ConstantFP *constantFloat) {
  auto floatValue = constantFloat->getValueAPF();

    // TODO: review the rules for mutation.
  if (floatValue.isZero()) {

    // TODO: Didn't find a better way of creating APFloat for number 1.
    APFloat replacementFloatValue = APFloat((double)1);
    ConstantFP *replacement =
      dyn_cast<ConstantFP>(ConstantFP::get(constantFloat->getContext(),
                                           replacementFloatValue));

    return replacement;
  }
  else {
    APFloat replacementFloatValue = APFloat::getZero(floatValue.getSemantics());
    ConstantFP *replacement =
      dyn_cast<ConstantFP>(ConstantFP::get(constantFloat->getContext(),
                                           replacementFloatValue));

    return replacement;
  }

  return nullptr;
}

llvm::Value *ScalarValueMutationOperator::applyMutation(Module *M,
                                                        MutationPointAddress address,
                                                        Value &_V) {

  llvm::Function &F = *(std::next(M->begin(), address.getFnIndex()));
  llvm::BasicBlock &B = *(std::next(F.begin(), address.getBBIndex()));
  llvm::Instruction &I = *(std::next(B.begin(), address.getIIndex()));

  for (unsigned int i = 0; i < I.getNumOperands(); i++) {
    Value *operand = I.getOperand(i);

    if (ConstantInt *constantInt = dyn_cast<ConstantInt>(operand)) {
      ConstantInt *replacement = getReplacementInt(constantInt);

      I.setOperand(i, replacement);

      return &I;
    }
    if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(operand)) {
      ConstantFP *replacement = getReplacementFloat(constantFloat);

      I.setOperand(i, replacement);

      return &I;
    }
  }

  llvm_unreachable("Must not reach here!");

  return nullptr;
}

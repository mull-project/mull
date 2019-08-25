#include "mull/Mutators/ScalarValueMutator.h"

#include "mull/Logger.h"
#include "mull/MutationPoint.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace llvm;
using namespace mull;

enum ScalarValueMutationType { None = 1, Int, Float };

const std::string ScalarValueMutator::ID = "scalar_value_mutator";
const std::string ScalarValueMutator::description =
    "Replaces zeros with 1, and non-zeros with 0";

#pragma mark - Prototypes

static std::pair<ScalarValueMutationType, Value *>
findPossibleApplication(Value &V);

static ConstantInt *getReplacementInt(ConstantInt *constantInt);
static ConstantFP *getReplacementFloat(ConstantFP *constantFloat);

#pragma mark - Implementations

static std::string getAsString(Value *numericOperandValue) {
  if (ConstantInt *constantInt = dyn_cast<ConstantInt>(numericOperandValue)) {
    const auto intValue = constantInt->getValue();
    return std::to_string(intValue.getSExtValue());
  }
  if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(numericOperandValue)) {
    const auto floatValue = constantFloat->getValueAPF();
    return std::to_string(floatValue.convertToDouble());
  }
  return "";
}

static std::string getAsReplacementString(Value *numericOperandValue) {
  if (ConstantInt *constantInt = dyn_cast<ConstantInt>(numericOperandValue)) {
    const auto replacementInt = getReplacementInt(constantInt);
    const auto replacementIntValue = replacementInt->getValue();
    return std::to_string(replacementIntValue.getSExtValue());
  }
  if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(numericOperandValue)) {
    const auto replacementFloat = getReplacementFloat(constantFloat);
    const auto replacementFloatValue = replacementFloat->getValueAPF();
    return std::to_string(replacementFloatValue.convertToDouble());
  }
  return "";
}

static std::string createDiagnostics(const std::string &originalString,
                                     const std::string &replacementString) {
  std::stringstream diagnostics;
  diagnostics << "Scalar Value Replacement: ";
  diagnostics << originalString;
  diagnostics << " -> ";
  diagnostics << replacementString;
  return diagnostics.str();
}

static std::pair<ScalarValueMutationType, Value *>
findPossibleApplication(Value &V) {
  Instruction *instruction = dyn_cast<Instruction>(&V);
  assert(instruction);

  unsigned opcode = instruction->getOpcode();

  if ((opcode >= Instruction::BinaryOpsBegin &&
       opcode < Instruction::BinaryOpsEnd) ||
      opcode == Instruction::Store || opcode == Instruction::FCmp ||
      opcode == Instruction::ICmp || opcode == Instruction::Ret ||
      opcode == Instruction::Call || opcode == Instruction::Invoke) {

    if (CallInst *callInstruction = dyn_cast<CallInst>(instruction)) {
      if (Function *callee =
              dyn_cast<Function>(callInstruction->getCalledValue())) {
        if (callee->getName().startswith("mull_")) {
          return std::make_pair(ScalarValueMutationType::None, nullptr);
        }
      }
    }

    for (Value *operand : instruction->operands()) {
      if (ConstantInt *constantInt = dyn_cast<ConstantInt>(operand)) {
        auto intValue = constantInt->getValue();

        /// Skip big number because getSExtValue throws otherwise.
        /// TODO: consider these edge cases in unit tests.
        if (intValue.getNumWords() > 1) {
          continue;
        }

        return std::make_pair(ScalarValueMutationType::Int, operand);
      }

      if (dyn_cast<ConstantFP>(operand)) {
        return std::make_pair(ScalarValueMutationType::Float, operand);
      }
    }
  }

  return std::make_pair(ScalarValueMutationType::None, nullptr);
}

static ConstantInt *getReplacementInt(ConstantInt *constantInt) {
  uint64_t replacementValue = constantInt->isZero() ? 1 : 0;

  APInt replacementIntValue =
      APInt(constantInt->getBitWidth(), replacementValue);

  ConstantInt *replacement = dyn_cast<ConstantInt>(
      ConstantInt::get(constantInt->getType(), replacementIntValue));

  return replacement;
}

static ConstantFP *getReplacementFloat(ConstantFP *constantFloat) {
  auto floatValue = constantFloat->getValueAPF();

  // TODO: review the rules for mutation.
  if (floatValue.isZero()) {

    // TODO: Didn't find a better way of creating APFloat for number 1.
    APFloat replacementFloatValue = APFloat((double)1);
    ConstantFP *replacement = dyn_cast<ConstantFP>(
        ConstantFP::get(constantFloat->getContext(), replacementFloatValue));

    return replacement;
  } else {
    APFloat replacementFloatValue = APFloat::getZero(floatValue.getSemantics());
    ConstantFP *replacement = dyn_cast<ConstantFP>(
        ConstantFP::get(constantFloat->getContext(), replacementFloatValue));

    return replacement;
  }
}

void ScalarValueMutator::applyMutation(Function *function,
                                       const MutationPointAddress &address) {
  llvm::Instruction &I = address.findInstruction(function);

  for (unsigned int i = 0; i < I.getNumOperands(); i++) {
    Value *operand = I.getOperand(i);

    if (ConstantInt *constantInt = dyn_cast<ConstantInt>(operand)) {
      ConstantInt *replacement = getReplacementInt(constantInt);

      I.setOperand(i, replacement);

      return;
    }
    if (ConstantFP *constantFloat = dyn_cast<ConstantFP>(operand)) {
      ConstantFP *replacement = getReplacementFloat(constantFloat);

      I.setOperand(i, replacement);

      return;
    }
  }

  llvm_unreachable("Must not reach here!");
}

std::vector<MutationPoint *>
ScalarValueMutator::getMutations(Bitcode *bitcode, llvm::Function *function) {
  assert(bitcode);
  assert(function);

  std::vector<MutationPoint *> mutations;

  for (auto &instruction : instructions(function)) {
    std::pair<ScalarValueMutationType, Value *> possibleApplication =
        findPossibleApplication(instruction);
    if (possibleApplication.first == ScalarValueMutationType::None) {
      continue;
    }

    std::string originalString = getAsString(possibleApplication.second);
    std::string replacementString =
        getAsReplacementString(possibleApplication.second);
    std::string diagnostics =
        createDiagnostics(originalString, replacementString);

    auto point = new MutationPoint(this, &instruction, diagnostics,
                                   replacementString, bitcode);
    mutations.push_back(point);
  }

  return mutations;
}

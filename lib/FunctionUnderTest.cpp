#include "mull/FunctionUnderTest.h"
#include "mull/Filters/InstructionFilter.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/ProfileData/Coverage/CoverageMapping.h>

#include <algorithm>

using namespace mull;

FunctionUnderTest::FunctionUnderTest(llvm::Function *function, Bitcode *bitcode)
    : function(function), bitcode(bitcode) {}

llvm::Function *FunctionUnderTest::getFunction() const {
  return function;
}

mull::Bitcode *FunctionUnderTest::getBitcode() const {
  return bitcode;
}

const std::vector<llvm::Instruction *> &FunctionUnderTest::getSelectedInstructions() const {
  return selectedInstructions;
}

void FunctionUnderTest::selectInstructions() {
  for (llvm::Instruction &instruction : llvm::instructions(function)) {
    if (SourceLocation::locationFromInstruction(&instruction).isNull())
      continue;
    selectedInstructions.push_back(&instruction);
  }
}

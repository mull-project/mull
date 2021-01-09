#include "mull/FunctionUnderTest.h"
#include "mull/Filters/InstructionFilter.h"

#include <llvm/IR/InstIterator.h>

using namespace mull;

FunctionUnderTest::FunctionUnderTest(llvm::Function *function, Bitcode *bitcode, bool covered)
    : function(function), bitcode(bitcode), covered(covered) {}

llvm::Function *FunctionUnderTest::getFunction() const {
  return function;
}

mull::Bitcode *FunctionUnderTest::getBitcode() const {
  return bitcode;
}

const std::vector<llvm::Instruction *> &FunctionUnderTest::getSelectedInstructions() const {
  return selectedInstructions;
}

bool FunctionUnderTest::isCovered() const {
  return covered;
}

void FunctionUnderTest::selectInstructions(const std::vector<InstructionFilter *> &filters) {
  for (llvm::Instruction &instruction : llvm::instructions(function)) {
    bool selected = true;
    for (InstructionFilter *filter : filters) {
      if (filter->shouldSkip(&instruction)) {
        selected = false;
        break;
      }
    }
    if (selected) {
      selectedInstructions.push_back(&instruction);
    }
  }
}

#include "mull/ReachableFunction.h"
#include "mull/Filters/InstructionFilter.h"

#include <llvm/IR/InstIterator.h>

mull::FunctionUnderTest::FunctionUnderTest(llvm::Function *function) : function(function) {}

llvm::Function *mull::FunctionUnderTest::getFunction() const {
  return function;
}

const std::vector<llvm::Instruction *> &mull::FunctionUnderTest::getSelectedInstructions() const {
  return selectedInstructions;
}

void mull::FunctionUnderTest::selectInstructions(const std::vector<InstructionFilter *> &filters) {
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

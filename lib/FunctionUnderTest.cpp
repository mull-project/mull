#include "mull/FunctionUnderTest.h"
#include "mull/Filters/InstructionFilter.h"

#include <llvm/IR/InstIterator.h>
#include <llvm/ProfileData/Coverage/CoverageMapping.h>

#include <algorithm>

using namespace mull;

FunctionUnderTest::FunctionUnderTest(llvm::Function *function, Bitcode *bitcode, bool covered, std::vector<llvm::coverage::CountedRegion> linecoverage)
    : function(function), bitcode(bitcode), covered(covered), linecoverage(linecoverage) {}

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

bool FunctionUnderTest::isCovered(const SourceLocation& location) const {
  if (linecoverage.size() == 0){
    return covered;
  }
  bool iscovered = true;
  auto after_start = [&location](auto LineStart, auto ColumnStart){
    return (LineStart < location.line || (LineStart == location.line && ColumnStart <= location.column));
  };
  auto before_end = [&location](auto LineEnd, auto ColumnEnd){
    return (LineEnd > location.line || (LineEnd == location.line && ColumnEnd >= location.column));
  };
  for_each(linecoverage.begin(), linecoverage.end(), [&](const auto & cov){
    if (after_start(cov.LineStart, cov.ColumnStart) && before_end(cov.LineEnd, cov.ColumnEnd)){
      iscovered &= cov.ExecutionCount > 0;
    }
  });
  return iscovered;
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

#pragma once

#include "mull/SourceLocation.h"
#include <vector>
#include <llvm/ProfileData/Coverage/CoverageMapping.h>

namespace llvm {
class Function;
class Instruction;
} // namespace llvm

namespace mull {
class InstructionFilter;
class Bitcode;

class FunctionUnderTest {
public:
  FunctionUnderTest(llvm::Function *function, Bitcode *bitcode);
  llvm::Function *getFunction() const;
  Bitcode *getBitcode() const;
  const std::vector<llvm::Instruction *> &getSelectedInstructions() const;
  void selectInstructions(const std::vector<InstructionFilter *> &filters);

private:
  llvm::Function *function;
  Bitcode *bitcode;
  std::vector<llvm::Instruction *> selectedInstructions;
};

} // namespace mull
